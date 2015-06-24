#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include "_get_key.h"
#include "com_op.h"
#include "pipe_op.h"
#include "logging.h"
#include "config.h"
#include "filedlg.h"
    
#define CONFIG_FN "comtty.cfg"
#define CMDPIPE "/tmp/comtty.pipe"
#define PACKSIZE 256
#define VERSION_NUMBER "1.4"
//#define MSG_INFO(fmt,...) printf("[INFO] "fmt, __VA_ARGS__)

#define MSG_INFO(fmt,...) do {\
    printf("\n\x1b[32m[INFO] "fmt,  ##__VA_ARGS__);\
    printf("\x1b[0m");\
}while(0)

#define MSG_ERR(fmt,...) do {\
    printf("\n\x1b[31m[ERR] "fmt, ##__VA_ARGS__);\
    printf("\x1b[0m");\
}while(0)

char *pversion[] = {
    "\n------Software Information------\n",
    "   Version: "VERSION_NUMBER"\n",
    "   Create date: 2007.02.16\n",
    "   Last update: 2015.06.24\n",
    "   Author: JEMYZHANG\n",
    "-------------------------------\n\n",
    NULL,
};

char *pmenu[] = {
    "\n-------------MENU-------------\n",
    "  1.Send file...(F1)\n",
    "  2.Start to record log...(F2)\n",
    "  3.Clear screen...(F3)\n",
    "  4.Run shell command...(F4)\n",
    "  5.Refresh Settings...(F5)\n",
    "  6.About me...\n",
    "  0.Exit...(Ctrl-Q)\n",
    "------------------------------\n",
    "Please select a command: ",
    NULL,
};

#define __parent_process_func__ 
#define __child_process_func__

const CONFIG_t default_config[] = {
    { "#", "# Config file for comtty[Ver 1.1]" },
    { "#", "# Configuration of com-port" },
    { "Baudrate", "115200" },
    { "DataBits", "8" },
    { "StopBit", "1" },
    { "Parity", "N" },
    { "#", "# Configuration of key shortcut" },
    { "#", "# F1 - F5 reserved for program function" },
    { "#", "# \"\\n\" -- 0x0d" },
    { "#", "# \"#!-\" sleep for 10ms" },
    { "#", "# \"#!|\" sleep for 1s" },
    { "#", "# \"#!~\" sleep for 5s" },
    { "#", "# \"#!!\"sleep for 10s" },
    { "F6", "\\n" },
    { "F7", "\\n" },
    { "F8", "\\n" },
    { "F9", "\\n" },
    { "F10", "\\n" },
    { "F11", "\\n" },
    { "F12", "\\n" },
#if 0
    { "UP", "2-4-\\n" },
    { "DOWN", "2-5-\\n" },
    { "LEFT", "2-7-\\n" },
    { "RIGHT", "2-6-\\n" },
#endif
    { "#", "# end of configure file" },
    {NULL,NULL},    
};

CONFIG_t config_g[100];

void _init_config_var(void)
{
    int i;
    for(i = 0; i< sizeof(config_g)/sizeof(CONFIG_t); i++)
    {
        config_g[i].key = NULL;
        config_g[i].value = NULL;    
    }    
}

int _create_default_config(void)
{
    int ret = 0,cnt = 0;
    while(default_config[cnt].key != NULL)
    {
            config_g[cnt].key = default_config[cnt].key;
            config_g[cnt].value = default_config[cnt].value;
            cnt ++;            
    }
    ret = save_config(CONFIG_FN,config_g,sizeof(config_g)/sizeof(CONFIG_t));
    return ret;
}

char *_get_configvalue(char *key)
{
    int i = 0;
    int pos = -1;
    while( config_g[i].key != NULL)
    {
        if(strcmp(config_g[i].key,key) == 0)
        {
            pos = i;
            break;
        }
        i ++;    
    }
    if(pos == -1 || strcmp(config_g[pos].value," ") == 0) return NULL;
    return config_g[pos].value;
}

void send_shortcuts(int device,char *key)
{
    char *shortcuts;
    shortcuts = _get_configvalue(key);
    if(shortcuts != NULL )
    {
        sendcmds(device,shortcuts);
    }
    return;
}

void disp_text(char **ptxt)
{
    char **p;
    p = ptxt;
    while(*p != NULL)
    {
        printf("%s",*p++);
    }
    return;    
}

void disp_dbg_menu(void)
{
    disp_text(pmenu);
    return;
}

void disp_version(void)
{
    disp_text(pversion);
    return;
}

void cmd_transfile(int device)
{
    char *pfilepath;
    
     pfilepath = fopenDlg("Transfer data","All(*.*)\0*.*\0",NULL);
     if( pfilepath == NULL)
     {
         MSG_INFO("Abort send file...\n");
     }else
     {
         MSG_INFO("Ready to send file: %s",pfilepath);
         if(sendfile(pfilepath,device,PACKSIZE) == -1)
         {
             MSG_ERR("Abort...Faild while open/sent file: %s\n",pfilepath);
         }
         else
         {
             MSG_INFO("Finished!\n");
         }
     }    
}

void cmd_logfile(PIPE_INFO_t *pipe_info)
{
    switch(pipe_info->log_switch)
     {
         case 0:
             pipe_info->log_path = fsaveDlg("Save Log","All(*.*)\0*.*\0Text(*.txt)\0*.txt\0","/tmp");
             if(pipe_info->log_path == NULL)
             {
                 MSG_INFO("Abort logging...\n");
             }
             else
             {
                 MSG_INFO("Save to log file: %s\n",pipe_info->log_path);
                 pipe_info->log_switch = 1;
                 create_log(pipe_info->log_path);
                 pipe_write(CMDPIPE,pipe_info);
                 MSG_INFO("Log started.\n");
             }
             break;
         case 1:
             pipe_info->log_switch = 0;
             pipe_write(CMDPIPE,pipe_info);
             MSG_INFO("Log stopped\n");
             break;
         default:
             break;
     }             
}

void cmd_shellmode(PIPE_INFO_t *pipe_info)
{
    char syscmd[1024];

    pipe_info->sig_cmdmode = 1;
    pipe_write(CMDPIPE,pipe_info);
    printf("\nEnter shell command mode:\n");
    printf("cmd@tty$ ");
    while(pipe_info->sig_cmdmode == 1)
    {
        _init_array(syscmd,sizeof(syscmd));
        if(_get_input_string(syscmd) == -1 || strcmp(syscmd,"exit") == 0)
        {
            printf("\nExit shell command mode...\n");
            pipe_info->sig_cmdmode = 0;
            pipe_write(CMDPIPE,pipe_info);
        }
        else
        {
            printf("\n");
            system(syscmd);
            pipe_info->sig_cmdmode = 1;
            printf("cmd@tty$ ");
            fflush(stdout);
        }
    }
}

void cmd_exit(PIPE_INFO_t *pipe_info)
{
                        pipe_info->sig_term = 1; 
                        pipe_write(CMDPIPE,pipe_info);                   
                        exit(0);
}

int __parent_process_func__ get_cmd(int device)
{
    char cmdbuf;
    PIPE_INFO_t *pipe_info;
    int ret;
    
    pipe_info = (PIPE_INFO_t *)malloc(sizeof(PIPE_INFO_t));
    pipe_info->log_switch = 0;
    pipe_info->sig_term = 0;
    pipe_info->sig_cmdmode = 0;
    pipe_info->log_path = NULL;
    if (pipe_write(CMDPIPE,pipe_info) == -1) 
    {
        MSG_ERR("Can not open pipe");
    }
         
    //system ("stty -F /dev/tty cbreak");
        
    while(1)
    {
        cmdbuf = _get_key(0,1);
#if 0
        if(cmdbuf == -1 && g_speckey != -1)
        {
            cmdbuf = g_speckey;
            g_speckey = -1;
        }
#endif
        switch(cmdbuf)
        {
            case 0x0a:
                sendcmd(device,0x0d);
                break;
            case 0x12: //Ctrl+R
                if(pipe_info->log_switch == 1) 
                {
                    pmenu[2] = "  2.Stop record log...(F2)\n";
                }else
                {
                    pmenu[2] = "  2.Start to record log...(F2)\n";
                }                
                disp_dbg_menu();
                switch(_get_input_num())
                {
                    case 0:
                        printf("\n");
                        cmd_exit(pipe_info);
                        break;
                    case 1:
                        cmd_transfile(device);
                        break;
                    case 2:
                        cmd_logfile(pipe_info);
                        break;
                    case 3:
                        printf("\33[2J");
                        break;
                    case 4:
                        cmd_shellmode(pipe_info);
                        break;
                    case 5:
                        ret = reload_config(CONFIG_FN, config_g,
                                                        sizeof(config_g)/sizeof(CONFIG_t));
                        MSG_INFO("%s reload configurations...\n",(ret != -1 ? "Successfully" : "Failed"));
                        break;
                    case 6:
                        disp_version();
                        break;
                    case -1:
        
                        MSG_INFO("Abort Command Mode...\n");
                        break;
                    default:
                        MSG_ERR("Command Error!\n");
                        break;
                }
                break;
            case KEYUP:
                sendcmds(device,"\x1b[A");
                //send_shortcuts(device,"UP");
                break;
            case KEYDW:
                sendcmds(device,"\x1b[B");
                //send_shortcuts(device,"DOWN");
                break;
            case KEYRT:
                sendcmds(device,"\x1b[C");
                //send_shortcuts(device,"RIGHT");
                break;
            case KEYLF:
                sendcmds(device,"\x1b[D");
                //send_shortcuts(device,"LEFT");
                break;
            case KEYF1:
                sendcmds(device,"1-\n-1-\n");            
                cmd_transfile(device);
                break;
            case KEYF2:
                cmd_logfile(pipe_info);
                break;
            case KEYF3:
                 printf("\33[2J");
                 break;
            case KEYF4:
                cmd_shellmode(pipe_info);
                break;
            case KEYF5:
                ret = reload_config(CONFIG_FN, config_g,
                                                sizeof(config_g)/sizeof(CONFIG_t));
                MSG_INFO("%s reload configurations...\n",(ret != -1 ? "Successfully" : "Failed"));
                break;
            case KEYF6:
                send_shortcuts(device,"F6");
                break;
            case KEYF7:
                send_shortcuts(device,"F7");
                break;
            case KEYF8:
                send_shortcuts(device,"F8");
                break;
            case KEYF9:
                send_shortcuts(device,"F9");
                break;
            case KEYF10:
                send_shortcuts(device,"F10");
                break;
            case KEYF11:
                send_shortcuts(device,"F11");
                break;
            case KEYF12:
                send_shortcuts(device,"F12");
                break;
            case KEYINS:
            case BKSPC:
                break;
            case KEYDEL:
                cmdbuf = 0;
                break;
            //case KEYESC:
            case 0x11: ///Ctrl+Q
                cmd_exit(pipe_info);
                break;
            case -1:
                break;
            default:
                sendcmd(device,cmdbuf);
                break;
        }
    }
    return 0;
}

int __child_process_func__ print_msg(int device)
{
    char msgbuf[512];
    pid_t pid;
    PIPE_INFO_t *pipe_info;
    
    pipe_info = (PIPE_INFO_t *)malloc(sizeof(PIPE_INFO_t));
    pipe_info->log_switch = 0;
    pipe_info->sig_term = 0;
    pipe_info->sig_cmdmode = 0;
    pipe_info->log_path = (char *)malloc(1024);
    
    pid = fork();
    if(pid < 0) MSG_ERR("Create pid error\n");
    else if(pid != 0)
    {
        while(1)
        {
            pipe_read(CMDPIPE,pipe_info);
            if(pipe_info->sig_term == 1)
            {
                free(pipe_info->log_path);
                free(pipe_info);
                kill(pid,9);
                 return(1);
            }
            sleep(1);
        }
    }
    else
    {        
       while(1)
       {
            _init_array(msgbuf,sizeof(msgbuf));        
            if(recvmsg(device,msgbuf,512) > 0)
            {
                pipe_read(CMDPIPE,pipe_info);
                if(msgbuf > 0 && pipe_info->sig_cmdmode == 0) printf("%s",msgbuf);
                fflush(stdout);
                if(pipe_info->sig_term == 1) return(1);
                
                if(pipe_info->log_switch == 1) 
                {
                    put_log(pipe_info->log_path,msgbuf, strlen(msgbuf));
                }
            }
        } 
   }

    return 0;
}

int main(int argc, char *argv[])
{
    pid_t pid;
    int fdevice;
    char *config;
    COM_CONFIG_t comconfig;
      
    char *pdevname;    ///dev/com
     
    if(argc < 2)
    {
       printf("Usage:\n    %s <device name>\t\tDevice name, such as com1.\n",argv[0]);
       exit(0);
    }
     
    pdevname = (char *)malloc(sizeof(argv[1]) + 5);
    strcpy(pdevname,"/dev/");
    strcat(pdevname,argv[1]);

    _init_config_var();
    if(load_config(CONFIG_FN, config_g,sizeof(config_g)/sizeof(CONFIG_t)) == -1)
    {
        _create_default_config();    
    }
    
    comconfig.baudrate = 115200;
    comconfig.databits = 8;
    comconfig.stopbit = 1;
    comconfig.parity = 'N';
    
    if((config =_get_configvalue("Baudrate")) != NULL)
    {
        comconfig.baudrate = atoi(config);
    }
    
    if((config = _get_configvalue("DataBits")) != NULL)
    {
        comconfig.databits = atoi(config);
    }

    if((config = _get_configvalue("StopBit")) != NULL)
    {
        comconfig.stopbit = atoi(config);
    }

    if((config = _get_configvalue("Parity")) != NULL)
    {
        comconfig.parity = (int)(*config);
    }

    fdevice = OpenDev(pdevname);
    if (fdevice>0)
    {
        if(set_speed(fdevice,comconfig.baudrate) < 0)
        {
            exit(0);
        }
    }
    else
    { 
        MSG_ERR("Can't Open Serial Port!\n");
        exit(0);
    }
    if (set_Parity(fdevice,comconfig.databits,comconfig.stopbit,comconfig.parity)== FALSE)
    { 
        MSG_ERR("Set Parity Error\n");
        exit(1);
    }

     pid=fork();
     if(pid < 0)
     {
        MSG_ERR("Abort...program corrupt!\n");
        exit(1);
     }
     else if(pid == 0)
     {
        printf("\33[2J");
        printf("\33[41m\33[32m Serial TTY Debuger ver %s \33[0m\n", VERSION_NUMBER);
        printf("\33[41m\33[32m Ctrl-R for Menu            \33[0m\n");
        print_msg(fdevice);
     }
     else
     {
        get_cmd(fdevice);
     }
     
     exit(0);

}
