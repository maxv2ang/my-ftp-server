#include "ace/Log_Msg.h"
#include "ace/OS.h"

std::string get_realpath(const std::string& path)
{
    char buf[4096] = {0};
    realpath(path.c_str(), buf);
    return std::string(buf);
}

bool is_dir_exist(const std::string& path)
{
    DIR* r = opendir(path.c_str());
    if (r != NULL) {
        closedir(r);
        return true;
    }
    return false;
}

bool startsWith(const std::string& str, const std::string prefix)
{
    return (str.rfind(prefix, 0) == 0);
}

bool get_size(const std::string file_path, size_t& size)
{
    struct stat st;
    if (ACE_OS::stat(file_path.c_str(), &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            size = st.st_size;
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("File size: %u bytes.\n"),
                       st.st_size));
            return true;
        } else {
            ACE_DEBUG((LM_INFO, "It is not a normal file.\n"));
        }
    } else {
        ACE_ERROR((LM_ERROR, "Cannot find the file\n"));
    }

    return false;
}

int rand_port()
{
    // initialize random seed
    ACE_OS::srand(static_cast<unsigned int>(ACE_OS::time(nullptr)));
    return ACE_OS::rand() % 10000 + 40000;
}

void get_mode(mode_t mode, char* buf)
{
    ACE_OS::memset(buf, '-', 10);
    switch (mode & S_IFMT) {
    case S_IFBLK: // block device
        buf[0] = 'b';
        break;
    case S_IFCHR: // character device
        buf[0] = 'c';
        break;
    case S_IFDIR: // directory
        buf[0] = 'd';
        break;
    case S_IFIFO: // FIFO
        buf[0] = 'f';
        break;
    case S_IFLNK: // symbolic link
        buf[0] = 'l';
        break;
    case S_IFSOCK: // socket
        buf[0] = 's';
        break;
    default:
        buf[0] = '-';
        break;
    }

    if (mode & S_IRUSR) {
        buf[1] = 'r';
    }
    if (mode & S_IWUSR) {
        buf[2] = 'w';
    }
    if (mode & S_IXUSR) {
        buf[3] = 'x';
    }
    if (mode & S_IRGRP) {
        buf[4] = 'r';
    }
    if (mode & S_IWGRP) {
        buf[5] = 'w';
    }
    if (mode & S_IXGRP) {
        buf[6] = 'x';
    }
    if (mode & S_IROTH) {
        buf[7] = 'r';
    }
    if (mode & S_IWOTH) {
        buf[8] = 'w';
    }
    if (mode & S_IXOTH) {
        buf[9] = 'x';
    }
}

void get_ls_line(const char* filename, struct stat* st, char* buf, char* tmp)
{
    ACE_OS::memset(tmp, 0, 1024);
    get_mode(st->st_mode, buf);
    ACE_OS::snprintf(tmp, 1024, "%s %4d %-8d %-8d %8d %.12s %s\r\n", buf,
                     (int)st->st_nlink, st->st_uid, st->st_gid,
                     (int)st->st_size, 4 + ctime(&st->st_mtime), filename);
}

int get_ls_data(const char* path, std::vector<std::string>& list)
{
    struct stat st;
    if (ACE_OS::stat(path, &st) != 0) {
        return -1;
    }

    char buf[10] = {0};
    char tmp[1024] = {0};

    if (S_ISREG(st.st_mode)) {
        get_ls_line(path, &st, buf, tmp);
        list.push_back(std::string(tmp));
    } else {
        DIR* dir = opendir(path);
        if (dir == nullptr) {
            ACE_ERROR((LM_ERROR, ACE_TEXT("Open dir failed!\n")));
            return -1;
        }
        struct dirent* de;
        while ((de = readdir(dir)) != nullptr) {
            const char* filename = de->d_name;
            if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
                continue;
            }
            std::string file_path =
                    std::string(path) + "/" + std::string(filename);
            if (ACE_OS::stat(file_path.c_str(), &st) == 0) {
                get_ls_line(filename, &st, buf, tmp);
                list.push_back(std::string(tmp));
            }
        }
        closedir(dir);
    }
    return 0;
}