#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <string>
#include <fstream>
#include "MainWindow.h"
#include "logic/quiz.h"
#include "cli.h"

int main(int argc, char *argv[]) {
    QCoreApplication::setOrganizationName("Madiwka");
    QCoreApplication::setApplicationName("MadExam");
    bool useGui = true;
    bool verbose = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "nogui" || arg == "--nogui" || arg == "-nogui") {
            useGui = false;
            break;
        } else if (arg == "verbose" || arg == "--verbose" || arg == "-verbose") {
            verbose = true;
        }
    }

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir); 
    QString dbFilePath = dataDir + "/db.txt";
    std::string dbPath = dbFilePath.toStdString();

    std::shared_ptr<QuestionDatabase> db = std::make_shared<QuestionDatabase>();

    try {
        db->readQuestionsFromFile(dbPath);
        std::cout << "Reading DB from: "<<dbPath;
    } catch (const std::exception& e) {
        if (verbose)
            std::cerr << "Failed to read question DB: " << e.what() << std::endl;
    }

    if (useGui) {
        QApplication app(argc, argv);
        MainWindow w(db);
        w.show();
        int result = app.exec();

        try {
            db->writeQuestionsToFile(dbPath);
        } catch (const std::exception& e) {
            if (verbose)
                std::cerr << "Failed to save question DB: " << e.what() << std::endl;
        }

        return result;
    } else {
        CLI cli(db);
        cli.run();

        try {
            db->writeQuestionsToFile(dbPath);
        } catch (const std::exception& e) {
            if (verbose)
                std::cerr << "Failed to save question DB: " << e.what() << std::endl;
        }

        return 0;
    }
}