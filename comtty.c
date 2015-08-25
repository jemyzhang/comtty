#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/inotify.h>
#include <sys/poll.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "_get_key.h"
#include "com_op.h"
#include "logging.h"
#include "config.h"
#include "filedlg.h"

#define CONFIG_FN "comtty.cfg"
#define PACKSIZE 256
#define VERSION_NUMBER "1.6.2"

#define MSG_INFO(fmt,...) do {\
    printf("\n\x1b[32m"fmt,  ##__VA_ARGS__);\
    printf("\x1b[0m");\
}while(0)

#define MSG_ERR(fmt,...) do {\
    printf("\n\x1b[31m"fmt, ##__VA_ARGS__);\
    printf("\x1b[0m");\
}while(0)

static int gs_shmid;

char *pversion[] = {
        "\n------Software Information------\n",
        "   Version: "VERSION_NUMBER"\n",
        "   Create date: 2007.02.16\n",
        "   Last update: 2015.08.25\n",
        "   Author: JEMYZHANG\n",
        "-------------------------------\n\n",
        NULL,
};

char *pmenu[] = {
        "\n---------COMTTY v."VERSION_NUMBER"---------\n",
        "  1.Send file...(F1)\n",
        "  2.Start to record log...(F2)\n",
        "  3.Clear screen...(F3)\n",
        "  4.Run shell command...(F4)\n",
        "  5.Refresh Settings...(F5)\n",
        "  6.About me...\n",
        "  0.Exit...\n",
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

void cmd_logfile(CTRL_INFO_t *ctrl_info)
{
    switch(ctrl_info->log_switch)
    {
        case 0:
        {
            char curdir[256];
            char *fpath;
            getcwd(curdir,sizeof(curdir));
            fpath = fsaveDlg("Save Log","*.* *.txt *.log",curdir);
            if(fpath == NULL) {
                MSG_INFO("Abort logging...\n");
            } else {
                strcpy(ctrl_info->log_path, fpath);
                MSG_INFO("Save to log file: %s\n",ctrl_info->log_path);
                ctrl_info->log_switch = 1;
                create_log(ctrl_info->log_path);
                MSG_INFO("Log started.\n");
            }
            break;
        }
        case 1:
            ctrl_info->log_switch = 0;
            MSG_INFO("Log stopped\n");
            break;
        default:
            break;
    }
}

void cmd_shellmode(CTRL_INFO_t *ctrl_info)
{
    char syscmd[1024];

    ctrl_info->sig_blockoutput = 1;
    MSG_INFO("\nEnter shell command mode:\n");
    printf("cmd@tty$ ");
    while(ctrl_info->sig_blockoutput == 1)
    {
        _init_array(syscmd,sizeof(syscmd));
        if(_get_input_string(syscmd) == -1 || strcmp(syscmd,"exit") == 0)
        {
            MSG_INFO("\nExit shell command mode...\n");
            ctrl_info->sig_blockoutput = 0;
        }
        else
        {
            printf("\n");
            system(syscmd);
            ctrl_info->sig_blockoutput = 1;
            printf("cmd@tty$ ");
            fflush(stdout);
        }
    }
}

int __parent_process_func__ input_processor(int device)
{
#define INPUT_BUF_SIZE 4096
    char* input_buf = (char *)malloc(INPUT_BUF_SIZE);
    int ret;
    CTRL_INFO_t *ctrl_info;
    ctrl_info = (CTRL_INFO_t *)shmat(gs_shmid, 0, 0);
    memset(ctrl_info, 0, sizeof(CTRL_INFO_t));

    while(!ctrl_info->sig_term) {
        int len = read_input_seq(0, 1, input_buf, INPUT_BUF_SIZE);
        if (len == 0) {
            continue;
        }
        char fake_key = input_buf[0];
        if(len > 1) fake_key = gen_fake_key(input_buf);
        switch(fake_key) {
            case KEYF1:
                cmd_transfile(device);
                break;
            case KEYF2:
                cmd_logfile(ctrl_info);
                break;
            case KEYF3:
                printf("\33[2J");
                break;
            case KEYF4:
                cmd_shellmode(ctrl_info);
                break;
            case KEYF5:
                ret = reload_config(CONFIG_FN, config_g,
                                    sizeof(config_g) / sizeof(CONFIG_t));
                MSG_INFO("%s reload configurations...\n", (ret != -1 ? "Successfully" : "Failed"));
                break;
            case KEYF6:
            case KEYF7:
            case KEYF8:
            case KEYF9:
            case KEYF10:
            case KEYF11:
            case KEYF12: {
                char shortcut_key[3] = {'F','0','\0'};
                shortcut_key[1] = '6' + KEYF6 - fake_key;
                send_shortcuts(device, shortcut_key);
                break;
            }
            case 0x12: //Ctrl+R
                ctrl_info->sig_blockoutput = 1;
                if (ctrl_info->log_switch == 1) {
                    pmenu[2] = "  2.Stop record log...(F2)\n";
                } else {
                    pmenu[2] = "  2.Start to record log...(F2)\n";
                }
                disp_dbg_menu();
                switch (_get_input_num()) {
                    case 0:
                        printf("\n");
                        ctrl_info->sig_term = 1;
                        break;
                    case 1:
                        cmd_transfile(device);
                        break;
                    case 2:
                        cmd_logfile(ctrl_info);
                        break;
                    case 3:
                        printf("\33[2J");
                        break;
                    case 4:
                        cmd_shellmode(ctrl_info);
                        break;
                    case 5:
                        ret = reload_config(CONFIG_FN, config_g,
                                            sizeof(config_g) / sizeof(CONFIG_t));
                        MSG_INFO("%s reload configurations...\n", (ret != -1 ? "Successfully" : "Failed"));
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
                ctrl_info->sig_blockoutput = 0;
                break;
            default:
                sendbytes(device, input_buf, len);
                break;
        }
    }
    shmdt(ctrl_info);
    return 0;
}

struct winsize g_tty_winsz;
void get_ttywinSize(void)
{
    ioctl(fileno(stdin), TIOCGWINSZ, &g_tty_winsz);
}
void ttywinSizeChanged(int sig)
{
    get_ttywinSize();
    return;
}

void check_report_screensize(char c, int dev)
{
    static char esc_seq_idx = 0;
    if(esc_seq_idx > 0)
    {
        switch(esc_seq_idx)
        {
            case 1:
                if(c == '[')
                {
                    esc_seq_idx ++;
                }else{
                    esc_seq_idx = 0;
                }
                break;
            case 2:
                if(c == '6')
                {
                    esc_seq_idx ++;
                }else{
                    esc_seq_idx = 0;
                }
                break;
            case 3:
                if(c == 'n')
                {
                    char sendbuf[64];
                    sprintf(sendbuf, "\x1b[%d;%dR",
                            g_tty_winsz.ws_row,
                            g_tty_winsz.ws_col);
                    sendbytes(dev, sendbuf, strlen(sendbuf));
                }
                esc_seq_idx = 0;
                break;
            default:
                esc_seq_idx = 0;
                break;
        }
    }else {
        if (c == '\x1b') {
            esc_seq_idx = 1;
        }
    }
    return;
}

int __child_process_func__ port_reader(int device)
{
    CTRL_INFO_t *ctrl_info;
    ctrl_info = (CTRL_INFO_t *)shmat(gs_shmid, 0, 0);
    memset(ctrl_info, 0, sizeof(CTRL_INFO_t));


    get_ttywinSize(); //get win size first
    signal(SIGWINCH, ttywinSizeChanged);

    int buf_fds[2];
    int buf_ena = 1; //enable block buffer
    if(pipe(buf_fds) < 0)
    {
       buf_ena = 0;
    }

    int pid=fork();
    if(pid < 0)
    {
        MSG_ERR("Abort...program corrupt!\n");
        exit(EXIT_FAILURE);
    }
    if(pid == 0)
    {
        if(buf_ena) {
            int flags = fcntl(buf_fds[0], F_GETFL);
            fcntl(buf_fds[0], F_SETFL, flags | O_NONBLOCK);
            close(buf_fds[1]); //close write
        }
        while(!ctrl_info->sig_term) {
            char c = 0;
            if (buf_ena && ctrl_info->sig_blockoutput == 0) {
                while (read(buf_fds[0], &c, 1) > 0) {
                    write(STDOUT_FILENO, &c, 1);
                }
            }
            usleep(100);
        }
        close(buf_fds[0]);
    }else {
        int log_fds[2];
        if(pipe(log_fds) < 0)
        {
            perror("log pipe()");
            exit(EXIT_FAILURE);
        }
        int log_pid = fork();
        if(log_pid < 0)
        {
            perror("log fork()");
            exit(EXIT_FAILURE);
        }
        if(log_pid == 0)
        {
            log_to_file(ctrl_info, log_fds);
            return 0;
        }
        close(log_fds[0]); //close log read
        while (!ctrl_info->sig_term) {
            char c = 0;
            if(buf_ena) {
                close(buf_fds[0]); //close read
            }
            if (readbytes(device, &c, 1) > 0) {
                if (ctrl_info->sig_blockoutput == 0) {
                    write(STDOUT_FILENO, &c, 1);
                } else {
                    if(buf_ena) {
                        //write to pipe
                        write(buf_fds[1], &c, 1);
                    }
                }
                check_report_screensize(c, device);

                if (ctrl_info->log_switch == 1) {
                    write(log_fds[1], &c, 1);
//                    put_log(ctrl_info->log_path, &c, 1);
                }
            }
            usleep(10);
        }
        close(log_fds[1]);
        close(buf_fds[1]);
        wait(NULL);
    }

    shmdt(ctrl_info);

    return 0;

}

void port_connection_monitor(const char* pdevname)
{
    int in_fd = inotify_init1(IN_NONBLOCK);
    if(in_fd < 0)
    {
        perror("inotify_init1\n");
        return;
    }

    int wd = inotify_add_watch(in_fd, pdevname, IN_DELETE_SELF|IN_ATTRIB);
    if(wd < 0)
    {
        fprintf(stderr, "can not watch %s\n", pdevname);
        perror("inotify_add_watch");
        close(in_fd);
        return;
    }

    struct pollfd fd;
    nfds_t nfds;
    int pollnum;
    fd.fd = in_fd;
    fd.events= POLLIN;
    nfds = 1;
    CTRL_INFO_t *ctrl_info = (CTRL_INFO_t *)shmat(gs_shmid, 0, 0);

    while(!ctrl_info->sig_term)
    {
        pollnum = poll(&fd, nfds, 100);
        if(pollnum == -1)
        {
            if(errno == EINTR)
                continue;
            perror("poll");
            close(in_fd);
            return;
        }
        if(pollnum > 0)
        {
            if(fd.events & POLLIN)
            {
                char buf[4096];
                const struct inotify_event *event;
                ssize_t len;
                char *ptr;
                while(!ctrl_info->sig_term)
                {
                    len = read(in_fd, buf, sizeof(buf));
                    if(len == -1 && errno != EAGAIN)
                    {
                        perror("read");
                        close(in_fd);
                        return;
                    }
                    if(len <= 0) break;
                    for(ptr = buf; ptr < buf + len;
                        ptr += sizeof(struct inotify_event) + event->len)
                    {
                        event = (const struct inotify_event *)ptr;
                        if(event->mask & IN_DELETE_SELF ||
                           event->mask & IN_ATTRIB)
                        {
                            fprintf(stderr, "\nSerial port deviced was removed!\n");
                            ctrl_info->sig_term = 1;
                        }
                    }
                }
            }
        }
    }
    close(in_fd);
    return;
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
        exit(EXIT_SUCCESS);
    }

    pdevname = (char *)malloc(strlen(argv[1]) + 6);
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

    fdevice = open(pdevname, O_RDWR | O_NONBLOCK);
    if (fdevice>0)
    {
        if(setup_serialport(fdevice,
                            comconfig.baudrate,
                            comconfig.databits,
                            comconfig.stopbit,
                            comconfig.parity) < 0)
        {
            perror("setup serial");
            close(fdevice);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        fprintf(stderr, "Can't Open Serial Port[%s]!\n",pdevname);
        perror("open");
        exit(EXIT_FAILURE);
    }

    gs_shmid = shmget(IPC_PRIVATE, sizeof(CTRL_INFO_t), IPC_CREAT|0600);
    if(gs_shmid < 0)
    {
        perror("shmget");
        close(fdevice);
        exit(EXIT_FAILURE);
    }

    pid=fork();
    if(pid < 0)
    {
        perror("fork");
        close(fdevice);
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        printf("\33[2J");
        printf("\33[41m\33[32m Serial TTY Debuger ver %s \33[0m\n", VERSION_NUMBER);
        printf("\33[41m\33[32m Ctrl-R for Menu            \33[0m\n");
        port_reader(fdevice);
    }
    else
    {
        int ppid = fork();
        if(ppid < 0)
        {
            perror("fork2");
            close(fdevice);
            exit(EXIT_FAILURE);
        }
        if(ppid == 0)
        {
            input_processor(fdevice);
        }else {
            port_connection_monitor(pdevname);
            kill(ppid, SIGTERM);
            wait(NULL);
        }
        if(fdevice >= 0) {
            close(fdevice);
        }
        wait(NULL);
    }

    exit(EXIT_SUCCESS);
}
