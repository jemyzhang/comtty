#include <string.h>
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

char _analysis_kcode(char *code)
{
    FNKY_t *fk = fnkey;
    
    while(fk->keycode != NULL && strcmp(code,fk->keycode) != 0 ) fk++;
    if(fk->keycode == NULL) return -1;
    else return fk->retval;    
}

/*
 * getch() -- a blocking single character input from stdin
 *
 * Returns a character, or -1 if an input error occurs
 *
 * Conditionals allow compiling with or without echoing of the input
 * characters, and with or without flushing pre-existing buffered input
 * before blocking.
 */
char _get_key( char echo, char flush )
{
    struct termios old_termios, new_termios;
    int OPTIONAL_ACTIONS;
    int            error, keylen;
    char           cbuf[10];
    
    fflush( stdout );
    tcgetattr( fileno(stdin), &old_termios );

    new_termios = old_termios;
    cfmakeraw(&new_termios);

    /*
     * raw mode, line settings
     */
    new_termios.c_lflag     &= ~ICANON;
    if(echo)
    /*
     * enable echoing the char as it is typed
     */
    new_termios.c_lflag     |=  ECHO;
    else
    /*
     * disable echoing the char as it is typed
     */
    new_termios.c_lflag     &= ~ECHO;
    if(flush)
    /*
     * use this to flush the input buffer before blocking for new input
     */
    OPTIONAL_ACTIONS = TCSAFLUSH;
    else
    /*
     * use this to return a char from the current input buffer, or block
     * if no input is waiting
     */
    OPTIONAL_ACTIONS = TCSANOW;
    /*
     * minimum chars to wait for
     */
    new_termios.c_cc[VMIN]   = 1;
    /*
     * minimum wait time, 1 * 0.10s
     */
    new_termios.c_cc[VTIME]  = 1;

    error = tcsetattr( fileno(stdin), OPTIONAL_ACTIONS, &new_termios );
     _init_array(cbuf,sizeof(cbuf));
    if ( 0 == error )
    {
        /*
         * get char from stdin
         */
        keylen  = read( 0, &cbuf, sizeof(cbuf) );
    }
    /*
     * restore old settings
     */
    error += tcsetattr( 0, OPTIONAL_ACTIONS, &old_termios );
    if(error == 0 && keylen > 0)
    {
        if(keylen == 1) return cbuf[0];
        return(_analysis_kcode(cbuf));
    }
    else
    {
        return( -1);
    }
}  /* end of getch */

int _get_input(char *pdst, char endflag, char sChar, char eChar)
{
    char buf;
    int cnt = 0, csr = 0;
    char ins_status = 0;
    int i;
    //system ("stty -F /dev/tty cbreak");
    fflush(stdout);
    while(1)//buf != endflag)
    {
        buf = _get_key(0,1);
        if(buf == endflag) break;
        
        if((sChar <= buf && buf <=eChar) )
        {
            if(csr != cnt && ins_status == 0)
            {
                    for(i = cnt + 1; i >= csr; i--)
                    {
                        *(pdst + i) = *(pdst + i -1);
                    }
                    
            }
            *(pdst + csr) = buf;
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
            switch(buf)
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
                     ins_status = ~ins_status & 1;
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
    //system ("stty -F /dev/tty -cbreak");
    if(cnt == 0) return -1;
    return 0;
}

int _get_input_string(char *pdst)
{
    return _get_input(pdst, '\r', ' ', 'z');
}

int _get_input_num(void)
{
    char buf[1024];
    if(_get_input(buf, '\r', '0', '9') == -1)
    return -1;
    else
    return(atoi(buf));
        
}

