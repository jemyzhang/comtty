#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/inotify.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>
#include <sys/types.h>
#include <pwd.h>
#include "_get_key.h"
#include "com_op.h"
#include "logging.h"
#include "config.h"
#include "filedlg.h"
#include "common.h"

#define CONFIG_FN "comtty.cfg"
#define PACKSIZE 256
#define VERSION_NUMBER "1.6.6"

static char gs_config_path[256];

static int gs_shmid;

char *pversion[] = {
        "\n------Software Information------\n",
        "   Version: "VERSION_NUMBER"\n",
        "   Create date: 2007.02.16\n",
        "   Last update: 2015.09.22\n",
        "   Author: JEMYZHANG\n",
        "-------------------------------",
        NULL,
};

char *pmenu[] = {
        "\n\33[42;37;1m--- SerialPort Tools v."VERSION_NUMBER" ---\33[0m\n",
        "  1.Send file...\n",
        "  2.Start to record log...\n",
        "  3.Clear screen...\n",
        "  4.Run shell command...\n",
        "  5.Refresh Settings...\n",
        "  6.Enable Timestamps...\n",
        "  7.About me...\n",
        "  0.Exit...\n",
        "------------------------------\n",
        "Please select a command: ",
        NULL,
};

#define MAX_CONFIG_SZ 64
static CONFIG_t config_g[MAX_CONFIG_SZ];

#define __parent_process_func__
#define __child_process_func__

void send_shortcuts(int device,char *key)
{
    char *shortcuts;
    shortcuts = config_getvalue(key, config_g);
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
        memset(syscmd,0,sizeof(syscmd));
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

int __parent_process_func__ input_processor(int argc, char* argv[])
{
#define INPUT_BUF_SIZE 4096
    char* input_buf = (char *)malloc(INPUT_BUF_SIZE);
    int ret;
    CTRL_INFO_t *ctrl_info;
    ctrl_info = (CTRL_INFO_t *)shmat(gs_shmid, 0, 0);
    int device = ctrl_info->device;

    while(!ctrl_info->sig_term) {
        int len = read_input_seq(0, 1, input_buf, INPUT_BUF_SIZE);
        if (len == 0) {
            continue;
        }
        char fake_key = input_buf[0];
        if(len > 1) fake_key = gen_fake_key(input_buf);
        switch(fake_key) {
            case KEYF1:
            case KEYF2:
            case KEYF3:
            case KEYF4:
            case KEYF5:
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
                    pmenu[2] = "  2.Stop record log...\n";
                } else {
                    pmenu[2] = "  2.Start to record log...\n";
                }
                if (ctrl_info->sig_timestamp == 1) {
                    pmenu[6] = "  6.Disable Timestamps...\n";
                } else {
                    pmenu[6] = "  6.Enable Timestamps...\n";
                }
                while(1) {
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
                            ret = reload_config(gs_config_path, config_g,
                                                MAX_CONFIG_SZ);
                            MSG_INFO("%s reload configurations...\n", (ret != -1 ? "Successfully" : "Failed"));
                            break;
                        case 6:
                            if (ctrl_info->sig_timestamp) {
                                ctrl_info->sig_timestamp = 0;
                                MSG_INFO("Disable Timestamps\n");
                            } else {
                                ctrl_info->sig_timestamp = 1;
                                MSG_INFO("Enable Timestamps\n");
                            }
                            break;
                        case 7:
                            disp_version();
                            continue;
                        case -1:
                            MSG_INFO("Abort Command Mode...\n");
                            break;
                        default:
                            MSG_ERR("Command Error!\n");
                            break;
                    }
                    ctrl_info->sig_blockoutput = 0;
                    break;
                }
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

int __child_process_func__ port_reader(int argc, char* argv[])
{
    CTRL_INFO_t *ctrl_info;
    ctrl_info = (CTRL_INFO_t *)shmat(gs_shmid, 0, 0);
    int device = ctrl_info->device;

    get_ttywinSize(); //get win size first
    signal(SIGWINCH, ttywinSizeChanged);

    int buf_fds[2];
    if(pipe(buf_fds) < 0)
    {
        perror("block output buffer: pipe()");
        exit(EXIT_FAILURE);
    }

    int pid=fork();
    if(pid < 0)
    {
        perror("port reader: fork()");
        exit(EXIT_FAILURE);
    }
    int len = strlen(argv[0]);
    if(pid == 0)
    {

        strncpy(argv[0], "com/blockbuffer", len);
        int flags = fcntl(buf_fds[0], F_GETFL);
        fcntl(buf_fds[0], F_SETFL, flags | O_NONBLOCK);
        close(buf_fds[1]); //close write
        while(!ctrl_info->sig_term) {
            char c = 0;
            if (ctrl_info->sig_blockoutput == 0) {
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
            strncpy(argv[0], "com/logger", len);
            log_to_file(ctrl_info, log_fds);
            return 0;
        }else {
            strncpy(argv[0], "com/reader", len);
            close(log_fds[0]); //close log read
            while (!ctrl_info->sig_term) {
                char c = 0;
                close(buf_fds[0]); //close read
                if (readbytes(device, &c, 1) > 0) {
                    char outbuf[128];
                    int buflen = 1;
                    outbuf[0] = c;
                    if (c == '\n' && ctrl_info->sig_timestamp) {
                        struct tm* tm_info;
                        struct timeval tv;

                        gettimeofday(&tv, NULL);
                        tm_info = localtime(&tv.tv_sec);
                        sprintf(&outbuf[1], "\x1b[37;44m[%04d-%02d-%02d %02d:%02d:%02d.%03ld]\x1b[0m ",
                                (1900 + tm_info->tm_year), tm_info->tm_mon, tm_info->tm_mday,
                                tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, tv.tv_usec / 1000 );
                        buflen = strlen(outbuf);
                    }
                    if (ctrl_info->sig_blockoutput == 0) {
                        write(STDOUT_FILENO, outbuf, buflen);
                    } else {
                        //write to pipe
                        write(buf_fds[1], outbuf, buflen);
                    }
                    check_report_screensize(c, device);

                    if (ctrl_info->log_switch == 1) {
                        write(log_fds[1], outbuf, buflen);
//                    put_log(ctrl_info->log_path, &c, 1);
                    }
                } else {
                    usleep(10);
                }
            }
            close(log_fds[1]);
            close(buf_fds[1]);
            wait(NULL);
        }
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
                            close(ctrl_info->device);
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

char *helptext[] =
{
        "-c,--config\t\tconfigure file path",
        "-g,--generate\t\tgenerate default configure file,\n\t\t\tif the configure file is not exists",
        "-p,--port\t\tport device",
        "-b,--bandrate\t\tbaundrate",
        "-d,--databits\t\tdatabits",
        "-s,--stopbit\t\tstopbit",
        "-r,--parity\t\tparity",
        "-l,--log\t\tstart log and log filepath",
        "-h,--help\t\tprint out this help text",
        NULL,
};

void help()
{
    char *p;
    int i = 0;
    while((p = helptext[i++]) != NULL)
    {
        printf("%s\n",p);
    }
}

struct option options[] =
        {
                {"generate", no_argument, 0, 'g'},
                {"config", required_argument,0,'c'},
                {"port", required_argument,0,'p'},
                {"baudrate", required_argument,0,'b'},
                {"databits", required_argument,0,'d'},
                {"stopbit", required_argument,0,'s'},
                {"parity", required_argument,0,'r'},
                {"log", required_argument,0,'l'},
                {"help", no_argument, 0,'h'},
                {0,0,0,0}
        };

int main(int argc, char *argv[])
{
    pid_t pid;
    int fdevice;
    char *val;
    COM_CONFIG_t config;
    char *cfgfn = NULL;
    int opt,opt_idx;
    char gen_config = 0;
    char arg_log = 0;
    char* arg_logpath;
    memset(&config,-1,sizeof(config));

    while((opt = getopt_long(argc,argv,"hgc:p:b:d:s:r:l:",options, &opt_idx)) != -1)
    {
        switch(opt)
        {
            case 'g':
                gen_config = 1;
                break;
            case 'c':
                cfgfn = optarg;
                break;
            case 'p':
                strcpy(config.portname,optarg);
                break;
            case 'b':
                config.baudrate = atoi(optarg);
                break;
            case 'd':
                config.databits = atoi(optarg);
                break;
            case 's':
                config.stopbit = atoi(optarg);
                break;
            case 'r':
                config.parity = optarg[0];
                break;
            case 'l':
                arg_log = 1;
                arg_logpath = optarg;
                break;
            case 'h':
            default:
                help();
                return 0;
        }
    }

    if(cfgfn == NULL) {
      const char *homedir;
      if((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
      }
      sprintf(gs_config_path, "%s/.config/%s", homedir, CONFIG_FN);
    } else {
      sprintf(gs_config_path, "%s", cfgfn);
    }

    //load config from file
    if(load_config(gs_config_path, config_g, MAX_CONFIG_SZ) == -1)
    {
        if(gen_config) {
            if( -1 == config_create_default(gs_config_path, config_g, MAX_CONFIG_SZ))
            {
                printf("Failed to generate the configure file: %s\n",gs_config_path);
                return 0;
            }
        }else{
            perror(gs_config_path);
            return 0;
        }
    }

    if(config.portname[0] == -1 &&
            (val =config_getvalue("PortName", config_g)) != NULL)
    {
        strcpy(config.portname, val);
    }

    if(config.baudrate == -1 &&
            (val =config_getvalue("Baudrate", config_g)) != NULL)
    {
        config.baudrate = atoi(val);
    }

    if(config.databits == -1 &&
            (val = config_getvalue("DataBits", config_g)) != NULL)
    {
        config.databits = atoi(val);
    }

    if(config.stopbit == -1 &&
            (val = config_getvalue("StopBit", config_g)) != NULL)
    {
        config.stopbit = atoi(val);
    }

    if(config.parity == -1 && (val = config_getvalue("Parity", config_g)) != NULL)
    {
        config.parity = (int)(*val);
    }

    fdevice = open(config.portname, O_RDWR | O_NONBLOCK);
    if (fdevice>0)
    {
        if(setup_serialport(fdevice,
                            config.baudrate,
                            config.databits,
                            config.stopbit,
                            config.parity) < 0)
        {
            close(fdevice);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        fprintf(stderr, "Can't Open Serial Port[%s]!\n", config.portname);
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

    CTRL_INFO_t *ctrl_info = (CTRL_INFO_t *)shmat(gs_shmid, 0, 0);
    memset(ctrl_info, 0, sizeof(CTRL_INFO_t));
    ctrl_info->device = fdevice;
    if(arg_log != 0)
    {
        ctrl_info->log_switch = 1;
        strcpy(ctrl_info->log_path,arg_logpath);
    }

    int len = strlen(argv[0]);
    pid=fork();
    if(pid < 0)
    {
        perror("fork");
        close(fdevice);
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
//        printf("\33[2J");
        printf("\33[42;37;1m SerialPort Tools ver %s \33[0m\n", VERSION_NUMBER);
        printf("\33[42;37m %s %d %d,%c,%d  \33[0m\n",
        config.portname, config.baudrate, config.databits, config.parity, config.stopbit);
        printf("\33[42;37m Ctrl-R for Menu            \33[0m\n");
        port_reader(argc,argv);
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
            strncpy(argv[0], "com/input", len);
            input_processor(argc,argv);
        }else {
            strncpy(argv[0], "com/monitor", len);
            port_connection_monitor(config.portname);
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
