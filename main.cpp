#include <QApplication>
#include <string>
#include <fstream>
#include "MainWindow.h"
#include "logic/quiz.h"
#include "cli.h"  

int main(int argc, char *argv[]) {
    
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
    
    
    std::shared_ptr<QuestionDatabase> db = std::make_shared<QuestionDatabase>();
    
    try {
        db->readQuestionsFromFile("db.txt");
    } catch (const std::exception& e) {
        
    }
    
    if (useGui) {
        // GUI Mode
        QApplication app(argc, argv);
        
        MainWindow w(db);
        w.show();
        
        int result = app.exec();
        
        
        try {
            db->writeQuestionsToFile("db.txt");
        } catch (const std::exception& e) {
            
        }
        
        return result;
    } else {
        CLI cli(db);
        cli.run();

        try {
            db->writeQuestionsToFile("db.txt");
        } catch (const std::exception& e) {
        }
        
        return 0;
    }
}