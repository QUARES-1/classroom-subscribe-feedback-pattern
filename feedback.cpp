#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FILENAME 20
#define FB_LENGTH 1000

typedef struct {
    char name[FILENAME];
    char feedbacks[FB_LENGTH];
    char timeStr[64];
    time_t time;
}fb;


int show_menus_admin_version();//展示菜单，也就是选择了反馈系统后要做的事情。
int show_menus_regular();//普通用户的反馈系统。
void initialize_fb_sys();//初始化反馈系统，实际上就是初始化一个时间,创建一个文件罢了.
void feedback();//输入反馈的函数，其参数是要反馈的教室名，需要保证反馈的教室在列表中
void show_all_feedbacks();//从文件中读取信息并且打印出来
void reset_feedbacks();//重置反馈文件
int has_new_feedback();//检查是否有新反馈
void delete_feedback();//删除指定的反馈
void export_feedbacks_to_csv();//导出到excel中


int testc();//检测教室,在名单中返回1，不在返回0


void show_feedbacks_on_classname();//以班级为单位展示反馈，便于统一处理。
void show_new_feedback();//展示新的反馈
void show_tips();//借教室的提示

//辅助函数
void save_checked_time();//保存查看反馈的时间(admin)。
time_t read_last_checked_time();//查看
void saveToFile(const char *filename,const fb *s);//把获得的结构体存放到文件中
void s_gets(char *s,int n);//gets
void f_gets(char *s,int n);//gets

int is_admin=1;

// admin version
int show_menus_admin_version() {
    int n;

    printf("===================================================\n");
    printf("Admin Feedback System\n");
    printf("===================================================\n");
    printf("1. Show all feedbacks\n");
    printf("2. Show feedbacks by classname\n");
    printf("3. Show new feedbacks since last check\n");
    printf("4. Clear all feedbacks\n");
    printf("5. Enter a new feedback\n");
    printf("6. Delete the selected feedback\n");
    printf("7. Export feedbacks to Excel (CSV)\n");
    printf("8. Quit\n");
    printf("===================================================\n");
    printf("Please enter your choice (1-8): ");

    scanf("%d", &n);

    return n;
}


// regular version
int show_menus_regular() {
    while (1) {
        int n;
        printf("===================================================\n");
        printf("Regular Feedback System\n");
        printf("===================================================\n");
        printf("1. Enter a feedback\n");
        printf("2. Quit\n");
        printf("===================================================\n");
        printf("Please enter your choice (1/2): ");

        if (scanf("%d", &n) != 1) {  // 检查输入是否为整数
            while (getchar() != '\n');  // 清除缓冲区
            printf("Invalid input. Please enter 1 or 2.\n");
            continue;  // 重新输入
        }

        while (getchar() != '\n');  // 清除缓冲区

        switch (n) {
            case 1: feedback(); break;
            case 2: return 0;
            default: printf("Invalid choice. Please try again.\n"); break;
        }
    }
}

int main() {
    int first=1;
    while (1) {
        int choice;
        printf("===================================================\n");
        if (has_new_feedback()&&first) {
            printf("There are new feedbacks since your last check!\n");
            first=0;
        } else {
            printf("No new feedbacks since your last check.\n");
            first=0;
        }

        if (is_admin) {
            choice = show_menus_admin_version();

            while(getchar()!='\n');

            switch (choice) {
                case 1: show_all_feedbacks(); break;
                case 2: show_feedbacks_on_classname(); break;
                case 3: show_new_feedback(); break;
                case 4: reset_feedbacks(); break;
                case 5: feedback(); break;
                case 6: delete_feedback(); break;
                case 7: export_feedbacks_to_csv(); break;
                case 8: return 0; // 退出
                default: printf("Invalid choice. Please try again.\n"); while(getchar()!='\n');break;
            }
        } else {
            if(!show_menus_regular()) return 0;
        }
    }
}

//该函数需要一个教室名字，他会创建一个结构体，把反馈的时间，教室名，反馈的内容存进去，然后以结构体的形式保存到文件中，没有返回值，出现bug会提示.

void feedback() {
    char classname[FILENAME];//要进行核对的教室名字
    fb fb_messages;
    time_t now = time(NULL);
    if (now == -1) {
        perror("Error getting current time");
        return;
    }

    printf("Please enter the classname: ");
    while(1) {
        if (!fgets(classname, FILENAME, stdin)) {
            printf("Error reading classname.\n");
            return;
        }

        classname[strcspn(classname, "\n")] = '\0'; // 移除换行符
        if(testc()) break;
        else {
            printf("The classname you entered isn't in the list!try again.\n");
            continue;
        }
    }

    printf("Please enter your feedback (max %d characters, enter '^' to quit):\n", FB_LENGTH - 1);
    s_gets(fb_messages.feedbacks, FB_LENGTH);
    if (strcmp(fb_messages.feedbacks, "^") == 0) {
        printf("Feedback canceled.\n");
        return;
    }

    fb_messages.time = now;
    strftime(fb_messages.timeStr, sizeof(fb_messages.timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));
    strncpy(fb_messages.name, classname, FILENAME - 1);
    fb_messages.name[FILENAME - 1] = '\0';

    saveToFile("feedback_stores", &fb_messages);
    printf("Feedback for classroom '%s' saved successfully at %s.\n", fb_messages.name, fb_messages.timeStr);

    system("pause");
}


//内部函数，用于把存放于临时结构体中的信息储存到文本文档中.
void saveToFile(const char *filename, const fb *s) {
    FILE *file = fopen(filename, "ab");
    if (file == NULL) {
        perror("Error opening file to save feedback");
        return;
    }
    if (fwrite(s, sizeof(fb), 1, file) != 1) {
        perror("Error writing feedback to file");
    }
    fclose(file);
}


//这个单独实现展示全部的反馈(admin only)
//
void show_all_feedbacks() {
    FILE *file = fopen("feedback_stores", "rb");
    fb cell;
    int first = 1;
    int number = 0;

    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    printf("Feedbacks in the file:\n");
    while (fread(&cell, sizeof(fb), 1, file) == 1) {
        if (!first) {
            printf("--------------------------------------------------------------------\n");
        }
        first = 0;
        number++;
        printf("Number:%d\nUpload time:%s\nClassname:%s\nFeedbacks:%s\n", number,cell.timeStr, cell.name, cell.feedbacks);
    }

    if (first == 1) {
        printf("No feedbacks in the file.\n");
    }

    fclose(file);

    save_checked_time();  // 更新查看时间

    system("pause");
}

//重置反馈文件
void reset_feedbacks() {
    char confirm;

    printf("Are you sure to reset all feedbacks?(y/n)\n");

    scanf(" %c", &confirm);

    if (confirm == 'y' || confirm == 'Y') {
        FILE *file = fopen("feedback_stores", "w");
        if (file == NULL) {
            perror("Error opening file");
            return;
        }
        fclose(file);
        printf("Feedbacks have been reset.\n");
    }
    else printf("Reset canceled.\n");

    system("pause");
}

//按照班级查找反馈，适合小型数据量。
//前提：输入要查找的班级。
void show_feedbacks_on_classname() {
    char classname[FILENAME];
    int first=1;
    int in=0;
    fb cell;

    printf("Please enter the classname:\n");

    scanf("%19s", classname);

    FILE *file = fopen("feedback_stores", "rb");

    if (file == NULL) {
        printf("Error opening file");
        return;
    }

    while (fread(&cell, sizeof(fb), 1, file)==1) {
        if(!strcmp(cell.name, classname)) {
            if (!first) {
                printf("--------------------------------------------------------------------\n");
            }
            first=0;
            in=1;
            printf("upload time:%s\nclassname:%s\nfeedbacks:%s\n", cell.timeStr, cell.name, cell.feedbacks);
        }
    }

    if (!in) printf("No feedbacks to this classroom.\n");

    fclose(file);

    system("pause");
}

//展示和上次相比新的反馈
void show_new_feedback() {
    time_t last_time = read_last_checked_time();
    FILE *file = fopen("feedback_stores", "rb");
    int first = 1;
    int in = 0;
    fb cell;

    if (file == NULL) {
        printf("Error opening file\n");
        return;
    }

    while (fread(&cell, sizeof(fb), 1, file) == 1) {
        if (cell.time > last_time) {
            if (!first) {
                printf("--------------------------------------------------------------------\n");
            }
            first = 0;
            in = 1;
            printf("upload time:%s\nclassname:%s\nfeedbacks:%s\n", cell.timeStr, cell.name, cell.feedbacks);
        }
    }

    if (!in) printf("No new feedbacks.\n");

    fclose(file);

    save_checked_time();  // 更新查看时间

    system("pause");
}

// 导出反馈数据到 CSV 文件，以便于在excel中打开
void export_feedbacks_to_csv() {
    FILE *file = fopen("feedback_stores", "rb");
    FILE *csvFile = fopen("feedbacks.csv", "w");
    fb cell;

    if (file == NULL) {
        perror("Error opening feedback file");
        return;
    }

    if (csvFile == NULL) {
        perror("Error creating CSV file");
        fclose(file);
        return;
    }

    // 写入 CSV 文件的标题行
    fprintf(csvFile, "Number,Upload Time,Classname,Feedbacks\n");

    int number = 0;
    while (fread(&cell, sizeof(fb), 1, file) == 1) {
        number++;
        // 按 CSV 格式写入数据
        fprintf(csvFile, "%d,\"%s\",\"%s\",\"%s\"\n", number, cell.timeStr, cell.name, cell.feedbacks);
    }

    fclose(file);
    fclose(csvFile);

    printf("Feedback data has been successfully exported to 'feedbacks.csv'.\n");

    system("pause");
}

// 删除反馈主函数
void delete_feedback() {
    FILE *file = fopen("feedback_stores", "rb");
    FILE *newFile;
    fb cell;
    int number = 0, n,first= 1;
    char ch;

    if (file == NULL) {
        perror("Error opening file. Make sure the file exists and you have permission to access it.");
        return;
    }

    // 统计反馈条数并展示
    printf("Feedbacks in the file:\n");
    while (fread(&cell, sizeof(fb), 1, file) == 1) {
        if (!first) {
            printf("--------------------------------------------------------------------\n");
        }
        first = 0;
        number++;
        printf("Number:%d\nUpload time:%s\nClassname:%s\nFeedbacks:%s\n", number,cell.timeStr, cell.name, cell.feedbacks);
    }

    if (number == 0) {
        printf("No feedbacks in the file.\n");
        fclose(file);
        return;
    }

    save_checked_time();

    do {
        printf("Please enter the number of the feedback you want to delete: ");
        while (scanf("%d", &n) != 1 || n > number || n < 1) {
            printf("Invalid input, please enter again:\n");
            while (getchar() != '\n'); // 清理缓冲区
        }

        // 将文件指针重置到文件开头，并再次读取，待读取到输入的序号则停止
        fseek(file, 0, SEEK_SET);
        printf("The feedback you want to delete is:\n");
        int i = 0;
        while (fread(&cell, sizeof(fb), 1, file) == 1) {
            i++;
            if (i == n) {
                printf("Number:%d\nUpload time:%s\nClassname:%s\nFeedbacks:%s\n",
                       i, cell.timeStr, cell.name, cell.feedbacks);
                break;
            }
        }

        printf("Right? (y/n): ");
        while (scanf(" %c", &ch) != 1 || !strchr("yYnN", ch)) {
            printf("Invalid input, please enter again (y/n):\n");
            while (getchar() != '\n');
        }
        if (ch == 'y' || ch == 'Y') break;
    } while (1);

    // 创建新文件写入剩余反馈
    newFile = fopen("feedback_stores_new", "wb");
    if (newFile == NULL) {
        perror("Error opening new file for writing");
        fclose(file);
        return;
    }

    // 重置文件指针，开始从头遍历并写入新文件，跳过删除的反馈
    fseek(file, 0, SEEK_SET);
    int i = 0;
    while (fread(&cell, sizeof(fb), 1, file) == 1) {
        i++;
        if (i != n) { // 如果不是要删除的反馈，则写入新文件
            if (fwrite(&cell, sizeof(fb), 1, newFile) != 1) {
                perror("Error writing feedback to new file");
                fclose(file);
                fclose(newFile);
                return;
            }
        }
    }

    fclose(file);
    fclose(newFile);

    // 删除原文件
    if (remove("feedback_stores") != 0) {
        perror("Error deleting original file");
        return;
    }

    // 重命名新文件为原文件名
    if (rename("feedback_stores_new", "feedback_stores") != 0) {
        perror("Error renaming new file to original filename");
        return;
    }

    printf("Feedback %d has been successfully deleted.\n", n);
}

// 函数：保存查看时间到文件,便于和下次比较.
void save_checked_time() {
    time_t now = time(NULL);
    FILE *file = fopen("last_checked_time.txt", "wb");

    if (file == NULL) {
        perror("Error opening file to save last checked time");
        return;
    }

    if (fwrite(&now, sizeof(time_t), 1, file) != 1) {
        perror("Error writing last checked time to file");
    }

    fclose(file);
}


// 函数：读取上次查看时间
time_t read_last_checked_time() {
    FILE *file = fopen("last_checked_time.txt", "rb");
    time_t lastCheckedTime = 0;  // 默认返回0，如果文件不存在或读取失败

    if (file != NULL) {
        if (fread(&lastCheckedTime, sizeof(time_t), 1, file) != 1) {
            perror("Error reading last checked time from file");
        }
        fclose(file);
    }

    return lastCheckedTime;
}


//在登陆时检查有没有新反馈
int has_new_feedback() {
    time_t last_time = read_last_checked_time(); // 读取上次查看时间
    FILE *file = fopen("feedback_stores", "rb");
    if (file == NULL) {
        printf("Error opening feedback file.\n");
        return 0; // 文件打开失败，认为没有新反馈
    }

    fb cell;
    int has_new = 0;
    while (fread(&cell, sizeof(fb), 1, file) == 1) {
        if (cell.time > last_time) {
            has_new = 1; // 存在新反馈
            break; // 找到一个新反馈后立即退出循环
        }
    }

    fclose(file);
    return has_new; // 返回是否有新反馈
}


//初始化反馈系统
void initialize_fb_sys() {
    save_checked_time();
    FILE *file = fopen("feedback_stores", "w");
    fclose(file);
}

//自定义读取文本，读到q时结束，需要封装起来
void s_gets(char *s,int n) {
    char ch;
    int i=0;
    while((ch=getchar())!=EOF&&i<n-1&&ch!='^') {
        s[i] = ch;
        i++;
    }
    s[i] = '\0';
}

//读取一行，不读取换行符（避免gets越界）
void f_gets(char *s,int n) {
    char ch;
    int i=0;
    while((ch=getchar())!=EOF&&i<n-1&&ch!='\n') {
        s[i] = ch;
        i++;
    }
    s[i] = '\0';
}
//在预约教室时给出提示
void show_tips() {
    printf("\n======================= Tips for Booking the Classroom =======================\n");

    printf("1. **Provide Feedback After Usage**\n");
    printf("   After using the classroom, inform the administration about any issues:\n");
    printf("   Example: The projector was not functioning during my session.\n");
    printf("\n-----------------------------------------------------------------------------\n");

    printf("2. **Suggestions for Improvement**\n");
    printf("   Share ideas for improving the classroom experience:\n");
    printf("   Example: Consider adding better lighting or more amenities.\n");
    printf("\n-----------------------------------------------------------------------------\n");

    printf("3. **Protect the Classroom**\n");
    printf("   - Follow usage guidelines (e.g., no eating or drinking).\n");
    printf("   - Avoid leaving permanent marks or sticking posters on walls.\n");
    printf("\n-----------------------------------------------------------------------------\n");

    printf("4. **Handle Equipment with Care**\n");
    printf("   Use projectors, whiteboards, and other equipment responsibly.\n");
    printf("\n-----------------------------------------------------------------------------\n");

    printf("5. **Maintain Cleanliness**\n");
    printf("   After your session, ensure:\n");
    printf("   - No trash is left behind. Use bins provided.\n");
    printf("   - Return chairs and desks to their original positions.\n");
    printf("\n-----------------------------------------------------------------------------\n");

    printf("6. **Respect Others**\n");
    printf("   - Leave on time to ensure the next user has access as scheduled.\n");
    printf("   - Avoid arriving late, which may disrupt other users.\n");
    printf("\n-----------------------------------------------------------------------------\n");

    printf("7. **Control Noise Levels**\n");
    printf("   Keep noise levels reasonable to avoid disturbing adjacent classrooms.\n");
    printf("\n-----------------------------------------------------------------------------\n");

    printf("8. **Use Resources Responsibly**\n");
    printf("   - Turn off lights, air conditioners, and projectors after use.\n");
    printf("   - Avoid wasting resources like markers or chalk.\n");
    printf("\n-----------------------------------------------------------------------------\n");

    printf("9. **Report Maintenance Needs**\n");
    printf("   Inform the facility manager promptly about any damages or maintenance needs:\n");
    printf("   Example: Broken chairs or flickering lights.\n");
    printf("\n=============================================================================\n");
    printf("Hope you have a nice experience using the classroom!\n");
}

