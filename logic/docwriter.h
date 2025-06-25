// #pragma once
// #include <string>
// #include <vector>
// #include <memory>
// #include <zip.h>
// #include <pugixml.hpp>
// #include "quiz.h"

// class DocxWriter {
// public:
//     DocxWriter();
//     ~DocxWriter();
//     bool createDocument(const std::string& filePath, const std::shared_ptr<QuizVariant>& quizVariant);

// private:
//     bool addFileToZip(zip_t* zip, const std::string& filename, const std::string& content);
//     std::string generateDocumentXml(const std::shared_ptr<QuizVariant>& quizvariant);
//     std::string generateContentTypesXml();
//     std::string generateRelsXml();
//     std::string generateDocumentRelsXml();
//     std::string generateStylesXml();
//     std::string generateSettingsXml();
//     std::string generateFontTableXml();
//     std::string generateCoreXml();
//     std::string generateAppXml();
//     void addOverride(pugi::xml_node& types, const std::string& partName, const std::string& contentType);
// };

#pragma once
#include <string>
#include <vector>
#include <memory>
#include "quiz.h"

class DocumentWriter {
public:
    DocumentWriter();
    ~DocumentWriter();
    bool createDocument(const std::string& filePath, const std::shared_ptr<QuizVariant>& quizVariant);

private:
    std::string generateHtmlDocument(const std::shared_ptr<QuizVariant>& quizvariant);
    std::string generateCss();
    std::string escapeHtml(const std::string& str);
    std::string getCurrentDate();
};