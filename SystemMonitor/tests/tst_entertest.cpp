#include <QtTest>
// add necessary includes here
#include <QDebug>
class enterTest : public QObject
{
    Q_OBJECT

public:
    enterTest();
    ~enterTest();

private slots:
    void checkTabs();
    void libCheck();
    int fileOpen();
};


enterTest::enterTest()
{

}

enterTest::~enterTest()
{

}


void enterTest::checkTabs()
{
    int mainTabs=1;
    QCOMPARE(mainTabs,1);
}

void enterTest::libCheck()
{
int actual=1;
QCOMPARE(fileOpen(),1);

}

int enterTest::fileOpen()
{
    FILE* file = fopen("/proc/stat", "r");
            if (file == NULL) {
                perror("Could not open stat file");
                return 0;
            }
    return 1;
}


QTEST_MAIN(enterTest)

#include "tst_entertest.moc"
