#include "TimeRecorderWin32.h"

int main() {
    TimeRecorder* timeRecorder = new TimeRecorderWin32(
        L"C:/Home/GitProject/TimeRecorderNewSol/TimeRecorder/Recorder.db");
    timeRecorder->exec();
}
