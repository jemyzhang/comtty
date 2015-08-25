#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include "_get_key.h"

typedef struct{
    char *keycode;
    char retval;
}FNKY_t;

FNKY_t fnkey[] ={
    {"\x1b[A",KEYUP},
    {"\x1b[B",KEYDW},
    {"\x1b[C",KEYRT},
    {"\x1b[D",KEYLF},
#if defined(linux)
    {"\x1bOP",KEYF1},
    {"\x1bOQ",KEYF2},
    {"\x1bOR",KEYF3},
    {"\x1bOS",KEYF4},
    {"\x1b[15~",KEYF5},
#else
    {"\x1b[[A",KEYF1},
    {"\x1b[[B",KEYF2},
    {"\x1b[[C",KEYF3},
    {"\x1b[[D",KEYF4},
    {"\x1b[[E",KEYF5},
#endif
    {"\x1b[17~",KEYF6},
    {"\x1b[18~",KEYF7},
    {"\x1b[19~",KEYF8},
    {"\x1b[20~",KEYF9},
    {"\x1b[21~",KEYF10},
    {"\x1b[23~",KEYF11},
    {"\x1b[24~",KEYF12},
    {"\x1b[2~",KEYINS},
    {"\x1b[3~",KEYDEL},
    {NULL,0},
}; 

char gen_fake_key(char *code)
{
    FNKY_t *fk = fnkey;
    
    while(fk->keycode != NULL && strcmp(code,fk->keycode) != 0 ) fk++;
    if(fk->keycode == NULL) return -1;
    else return fk->retval;    
}

int read_input_seq(char echo, char flush, char *cbuf, size_t size)
{
    struct termios old_termios;
    struct termios termios;
    int OPTIONAL_ACTIONS;
    int error, keylen = 0;

    fflush( stdout );

    tcgetattr(fileno(stdin), &old_termios );

    termios = old_termios;

    /*
     * raw mode, line settings
     */
    cfmakeraw(&termios);
    termios.c_iflag |= (ICRNL);
    termios.c_oflag |= (OPOST|ONLCR);

    if(echo)
    {
        /*
         * enable echoing the char as it is typed
         */
        termios.c_lflag |=  ECHO;
    } else{
        /*
         * disable echoing the char as it is typed
         */
        termios.c_lflag &= ~ECHO;
    }
    if(flush)
    {
        /*
         * use this to flush the input buffer before blocking for new input
         */
        OPTIONAL_ACTIONS = TCSAFLUSH;
    } else{
        /*
         * use this to return a char from the current input buffer, or block
         * if no input is waiting
         */
        OPTIONAL_ACTIONS = TCSANOW;
    }
    /*
     * minimum chars to wait for
     */
    termios.c_cc[VMIN] = 1;
    /*
     * minimum wait time, 1 * 0.10s
     */
    termios.c_cc[VTIME] = 1;

    error = tcsetattr( fileno(stdin), OPTIONAL_ACTIONS, &termios );

    keylen = 0;
    if ( 0 == error )
    {
        /*
         * get char from stdin
         */
        keylen  = read( 0, cbuf, size );
    }
    /*
     * restore old settings
     */
    tcsetattr( fileno(stdin), OPTIONAL_ACTIONS, &old_termios );
    return keylen;
}

int _get_input(char *pdst, char endflag, char sChar, char eChar)
{
    char ch;
    int cnt = 0, csr = 0;
    int ins_status = 0;
    int i;
    char buf[5];
    while(1)
    {
        int len = read_input_seq(0, 1, buf, 5);
        if(len <= 0) continue;
        if(len > 1){
            ch = gen_fake_key(buf);
        } else{
            ch = buf[0];
        }
        if(ch == endflag) break;
        
        if((sChar <= ch && ch <=eChar) )
        {
            if(csr != cnt && ins_status == 0)
            {
                    for(i = cnt + 1; i >= csr; i--)
                    {
                        *(pdst + i) = *(pdst + i -1);
                    }
                    
            }
            *(pdst + csr) = ch;
            csr ++;
            if(ins_status == 0 || csr >= cnt) cnt++;
            *(pdst + cnt) = '\0';
            if(csr == 1 && csr == cnt) printf("%s",pdst);
            else
            {
                if(csr != cnt)
                {
                    if(csr != 1) printf("\33[s\33[%dD\33[K%s\33[u\33[C",csr - 1,pdst);
                    else  printf("\33[s%s\33[u\33[C",pdst);
                }  
                else
                    if(csr != 1) printf("\33[%dD\33[K%s",csr - 1,pdst);
                    else printf("%s",pdst);
            }            
        }
        else
        {
            switch(ch)
            {
                case BKSPC:
                case '\x7f':
                    if(csr > 0)
                    {
                        csr --;
                        cnt --;
                        for(i = csr; i < cnt; i ++)
                        {
                            *(pdst + i) = *(pdst + i + 1); 
                        }
                        *(pdst + cnt) = '\0';
                        if(csr != cnt)
                        {
                           printf("\33[s\33[%dD\33[K%s\33[u\33[D",csr+1,pdst);
                        }  
                        else
                           printf("\33[%dD\33[K%s",csr+1,pdst);
                    }
                    break;
                case KEYDEL:
                    break;
                case KEYINS:
                     ins_status = ins_status > 0 ? 0 : 1;
                     break;              
                case KEYLF:
                    if(csr >= 1) {csr --; printf("\33[D");};
                    break;
                case KEYRT:
                    if(csr < cnt) {csr ++;printf("\33[C");};
                    break;
                case KEYUP:
                    break;
                case KEYDW:
                    break;
                case KEYF1:
                default:
                    break;
             }
        }
        fflush(stdout);
    }
    *(pdst + cnt) = '\0';
    if(cnt == 0) return -1;
    return 0;
}

int _get_input_string(char *pdst)
{
    return _get_input(pdst, '\n', ' ', 'z');
}

int _get_input_num(void)
{
    char buf[1024];
    if(_get_input(buf, '\n', '0', '9') == -1)
    return -1;
    else
    return(atoi(buf));
        
}

