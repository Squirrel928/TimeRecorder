#include "TimeRecorderSQLite.h"
#include <iostream>

class Test : public TimeRecorderBase {
public:
    static void TestSQLite();
};

int main() { Test::TestSQLite(); }

void Test::TestSQLite() {
    TimeRecorderSQL* timeRecorder = new TimeRecorderSQLite(
        L"C:/Home/GitProject/TimeRecorderNewSol/TimeRecorder/Recorder.db");
    int choose;
    std::string name, exeName;
    TimeRecorderBase::TSize type, parType, mode, appId, begin, end, total;
    while (true) {
        printf("1. insert application\n");
        printf("2. delete application\n");
        printf("3. modify application\n");
        printf("4. insert type\n");
        printf("5. delete type\n");
        printf("6. modify type\n");
        printf("7. insert record\n");
        printf("8. delete record\n");
        printf("9. show all application\n");
        printf("10. show all type info\n");
        printf("11. quit\n");
        printf("choose: ");
        std::cin >> choose;
        switch (choose) {
        case 1:
            printf("name exeName type(int) runMode(int)\n");
            std::cin >> name >> exeName >> type >> mode;
            timeRecorder->AddApplication(ANSIToUnicode(name),
                                         ANSIToUnicode(exeName), type,
                                         (TimeRecorderBase::TMODE)mode);
            break;
        case 2:
            printf("appId\n");
            std::cin >> appId;
            timeRecorder->DelApplication(appId);
            break;
        case 3:
            printf("appId(int) name exeName type(int) runMode(int)\n");
            std::cin >> appId >> name >> exeName >> type >> mode;
            timeRecorder->ModifyApplication(appId, ANSIToUnicode(name),
                                            ANSIToUnicode(exeName), type,
                                            (TimeRecorderBase::TMODE)mode);
            break;
        case 4:
            printf("name parentType(int)\n");
            std::cin >> name >> parType;
            timeRecorder->AddType(ANSIToUnicode(name), parType);
            break;
        case 5:
            printf("typeId(int)\n");
            std::cin >> type;
            timeRecorder->DelType(type);
            break;
        case 6:
            printf("typeId(int) name subtype(int)\n");
            std::cin >> type >> name >> parType;
            timeRecorder->ModifyType(type, ANSIToUnicode(name), parType);
            break;
        case 7:
            printf("appId begin end total\n");
            std::cin >> appId >> begin >> end >> total;
            timeRecorder->AddRecord(
                appId, TimeRecorderBase::TRecord(begin, end, total));
            break;
        case 8:
            printf("appId begin end total\n");
            std::cin >> appId >> begin >> end >> total;
            timeRecorder->DelRecord(
                appId, TimeRecorderBase::TRecord(begin, end, total));
            break;
        case 9:
            printf("-------------applications----------------\n");
            printf("id, name, exeName, type, mode\n");
            for (auto i : timeRecorder->m_apps) {
                printf(
                    "%llu, %s, %s, %s, %s\n", i.sqlId,
                    UnicodeToANSI(i.name).c_str(),
                    UnicodeToANSI(i.exeName).c_str(),
                    UnicodeToANSI(timeRecorder->m_types[i.type].name).c_str(),
                    i.mode == TimeRecorderBase::TMODE::back ? "back" : "fore");
            }
            printf("-------------applications----------------\n");
            break;
        case 10:
            printf("-------------types------------\n");
            printf("id, name, parentType\n");
            for (auto i : timeRecorder->m_types) {
                printf("%llu, %s, %s\n", i.second.id,
                       UnicodeToANSI(i.second.name).c_str(),
                       i.second.supertype == 0
                           ? "null"
                           : UnicodeToANSI(
                                 timeRecorder->m_types[i.second.supertype].name)
                                 .c_str());
            }
            printf("-------------types------------\n");
            break;
        default:
            exit(0);
            break;
        }
    }
}
