#include "docwriter.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>

DocumentWriter::DocumentWriter() {}
DocumentWriter::~DocumentWriter() {}

bool DocumentWriter::createDocument(const std::string& filePath, const std::shared_ptr<QuizVariant>& quizvariant) {
    try {
        std::ofstream htmlFile(filePath);
        if (!htmlFile) {
            std::cerr << "Failed to create HTML file: " << filePath << std::endl;
            return false;
        }
        
        htmlFile << generateHtmlDocument(quizvariant);
        htmlFile.close();
        
        std::cout << "HTML document created successfully: " << filePath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error creating HTML document: " << e.what() << std::endl;
        return false;
    }
}

std::string DocumentWriter::generateHtmlDocument(const std::shared_ptr<QuizVariant>& quizvariant) {
    std::stringstream html;
    
    html << "<!DOCTYPE html>\n"
         << "<html lang=\"en\">\n"
         << "<head>\n"
         << "    <meta charset=\"UTF-8\">\n"
         << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
         << "    <title>" << escapeHtml(quizvariant->variantName) << "</title>\n"
         << "    <style>\n"
         << generateCss()
         << "    </style>\n"
         << "</head>\n"
         << "<body>\n";
    
    html << "    <div class=\"title\">" << escapeHtml(quizvariant->variantName) << "</div>\n";
    
    html << "    <div class=\"quiz-container\">\n";
    int questionNum = 1;
    
    std::string currentTopic = "";
    
    for (const auto& question : quizvariant->questions) {
        if (currentTopic != question->topic->name) {
            html << "        <div class=\"topic-section\">\n"
                 << "            <h2 class=\"topic-title\">" << escapeHtml(question->topic->name) << "</h2>\n"
                 << "        </div>\n";
                 
            currentTopic = question->topic->name;
        }
        
        html << "        <div class=\"question\">\n"
             << "            <div class=\"question-text\">Q" << questionNum << ": " 
             << escapeHtml(question->questionText) << "</div>\n";
        
        if (question->options) {
            html << "            <div class=\"options\">\n";
            char optionLetter = 'A';
            for (const auto& option : *question->options) {
                html << "                <div class=\"option\">" 
                     << optionLetter << ") " << escapeHtml(option) << "</div>\n";
                optionLetter++;
            }
            html << "            </div>\n";
        }
        
        html << "        </div>\n";
        questionNum++;
    }
    html << "    </div>\n";
    
    html << "    <div class=\"footer\">Сгенерировано MadExam " << getCurrentDate() << "</div>\n";
    
    html << "</body>\n"
         << "</html>";
    
    return html.str();
}

std::string DocumentWriter::generateCss() {
    return R"(
        body {
            font-family: 'Calibri', 'Segoe UI', Arial, sans-serif;
            line-height: 1.6;
            color: #333;
            margin: 0;
            padding: 20px;
            background-color: #f9f9f9;
        }
        .title {
            font-size: 24pt;
            font-weight: bold;
            text-align: center;
            margin-bottom: 30px;
            color: #2c3e50;
            border-bottom: 2px solid #3498db;
            padding-bottom: 10px;
        }
        .quiz-container {
            max-width: 800px;
            margin: 0 auto;
            background-color: white;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            padding: 20px;
        }
        .topic-section {
            margin-top: 30px;
            margin-bottom: 20px;
            background-color: #f8f9fa;
            border-left: 5px solid #3498db;
            padding: 10px 15px;
            page-break-before: always;
        }
        .topic-title {
            margin: 0;
            color: #2980b9;
            font-size: 16pt;
            font-weight: bold;
        }
        .question {
            margin-bottom: 25px;
            padding-bottom: 20px;
            border-bottom: 1px solid #eee;
        }
        .question-text {
            font-weight: bold;
            margin-bottom: 10px;
            font-size: 12pt;
        }
        .options {
            margin-left: 20px;
        }
        .option {
            margin-bottom: 8px;
            font-size: 11pt;
        }
        .footer {
            text-align: center;
            margin-top: 30px;
            font-size: 10pt;
            color: #7f8c8d;
        }
        @media print {
            body {
                background-color: white;
            }
            .quiz-container {
                box-shadow: none;
            }
            .topic-section {
                background-color: white;
                border-left-color: #666;
            }
        }
    )";
}

std::string DocumentWriter::escapeHtml(const std::string& str) {
    std::string escaped = str;
    size_t pos = 0;
    while ((pos = escaped.find('&', pos)) != std::string::npos) {
        escaped.replace(pos, 1, "&amp;");
        pos += 5; 
    }
    
    pos = 0;
    while ((pos = escaped.find('<', pos)) != std::string::npos) {
        escaped.replace(pos, 1, "&lt;");
        pos += 4; 
    }
    
    pos = 0;
    while ((pos = escaped.find('>', pos)) != std::string::npos) {
        escaped.replace(pos, 1, "&gt;");
        pos += 4; 
    }
    
    pos = 0;
    while ((pos = escaped.find('"', pos)) != std::string::npos) {
        escaped.replace(pos, 1, "&quot;");
        pos += 6; 
    }
    
    return escaped;
}

std::string DocumentWriter::getCurrentDate() {
    std::time_t now = std::time(nullptr);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%B %d, %Y", std::localtime(&now));
    return std::string(buf);
}