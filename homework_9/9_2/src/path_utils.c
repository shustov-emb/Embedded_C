#include <string.h>
#include <unistd.h>     // Для chdir и getcwd
#include <dirent.h>     // Для DIR, dirent, opendir, readdir
#include <sys/stat.h>   // Для stat и struct stat
#include <fcntl.h>      // Для open
#include "path_utils.h"


bool IsRootPath(const char *path) {
    //Простая проверка строки с символом /
    return path != NULL && strcmp(path, "/") == 0;
}

void BuildPath(const char *base, const char *name, char *out, size_t out_size) {
    
    //Если базовый путь корень - его и пишем иначе выводим мазовый путь+имя добавляемого объекта
    if (IsRootPath(base)) {
        snprintf(out, out_size, "/%s", name);
    } else {
        snprintf(out, out_size, "%s/%s", base, name);
    }
}

void BuildParentPath(const char *path, char *out, size_t out_size) {
   
    //Проверка на корень
    if (IsRootPath(path)) {
        snprintf(out, out_size, "/");
        return;
    }

    strncpy(out, path, out_size - 1);
    out[out_size - 1] = '\0';

    //Ищем первый слеш с конца строки, и если находим убираем
    size_t len = strlen(out);
    while (len > 1 && out[len - 1] == '/') {
        out[len - 1] = '\0';
        len--;
    }

    //Ищем слеш в буфере, если не нашли или слеш равен первому символу, то тогда это корень
    //Иначе просто отсекаем хвост, путь к родительскому каталогу
    char *last_slash = strrchr(out, '/');
    if (last_slash == NULL || last_slash == out) {
        snprintf(out, out_size, "/");
    } else {
        *last_slash = '\0';
    }
}

 // Просто заполняем данные панели объектами по новому пути
void RenewDirData(Panel *panel, const char *new_path) {
    FileData *new_data = NULL;
    int count = 0;

   
    if (new_path == NULL) {
        count = GetDirInfo(panel->current_dir, &new_data);
    } else {
        count = GetDirInfo((char *)new_path, &new_data);
    }

    if (count < 0) {
        free(new_data);
        return;
    }

    free(panel->dir_data);
    panel->dir_data = new_data;
    panel->files_count = (size_t)count;
    panel->top_index = 0;
    panel->selected_index = 0;

    if (new_path != NULL) {
        strncpy(panel->current_dir, new_path, sizeof(panel->current_dir) - 1);
        panel->current_dir[sizeof(panel->current_dir) - 1] = '\0';
    }
}

int GetDirInfo(char *path, FileData **records){
   
    struct dirent **entry;
    struct stat info;
    int desc = open(path, O_RDONLY);                  if(desc == -1 ) return -1;
    int n = scandir(path, &entry, NULL, alphasort);   if(n == -1) {close(desc); return -1; }

    //Выделяем память под массив структур FlieData, на размер с количество объектов в  каталоге
    *records = malloc((size_t)n * sizeof(FileData));
    
    //Если что-то пошло не так, высвобождаем память, закрываем дескриптор возвращаем -1
    if (*records == NULL) {
        for (int i = 0; i < n; i++) free(entry[i]);
        free(entry);
        close(desc);
        return -1;
    }

    //Переменная в которой будет хранить количество файлов
    int k = 0;
    if (n > 0)
    {
        for (size_t i = 0; i < (size_t)n; i++)
        {
            // фильтр как в mc - "." не показываем нигде, ".." скрываем только в корне 
            if (strcmp(entry[i]->d_name, ".") == 0 || (IsRootPath(path) && strcmp(entry[i]->d_name, "..") == 0)) {
                free(entry[i]);
                continue;
            }

            // Если успешно получили данные по объекту, смело записываем данные в наш массив структур
            if (fstatat(desc, entry[i]->d_name, &info, 0) == 0) {

                strncpy((*records)[k].name, entry[i]->d_name, sizeof((*records)[k].name) - 1);
                (*records)[k].name[sizeof((*records)[k].name) - 1] = '\0';
                (*records)[k].size = info.st_size;
                (*records)[k].is_dir = S_ISDIR(info.st_mode);
                k++;
            }

            //Экспиременты показали что dirent надо высвобождать, сам он этого не делает
            free(entry[i]);
        }
    }

    free(entry);
    close(desc);

    /*Поскольку каталоги "." или "..", мы можем скипнуть в зависимости от условий, выше, то перераспределяем выделенную память
     * под точное количество файлов которое мы в итоге получили*/
    FileData *shrunk = realloc(*records, k * sizeof(FileData));
    if (shrunk != NULL || k == 0) {
        *records = shrunk;
    }

    return k;
}

