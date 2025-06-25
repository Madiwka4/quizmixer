# include "cli.h"
# include "logic/quiz.h"
#include <algorithm>
#include <limits>
#include <sstream>
#include <random>


CLI::CLI(std::shared_ptr<QuestionDatabase> database) : db(database) {}

void CLI::run() {
    std::string command;
    std::cout << "Welcome to the Quiz CLI! Type 'help' for a list of commands.\n";
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, command);
        processCommand(command);
    }
}
void CLI::processCommand(const std::string& command) {
    if (command == "help") {
        showHelp();
    } else if (command == "add_topic") {
        addTopic();
    } else if (command == "remove_topic") {
        removeTopic();      
    } else if (command == "select_topic") {
        selectTopic();
    } else if (command == "list_topics") {
        listTopics();
    } else if (command == "add_question") {
        addQuestion();
    } else if (command == "remove_question") {
        removeQuestion();
    } else if (command == "generate_quiz") {    
        generateQuiz();
    } else if (command == "list_questions") {
        listQuestions();
    } else if (command == "exit") {
        exit();
    } else {
        std::cout << "Unknown command: " << command << ". Type 'help' for a list of commands.\n";
    }
}

void CLI::showHelp(){
    std::cout << "Available commands:\n"
              << "  help - Show this help message\n"
              << "  add_topic - Add a new topic\n"
              << "  remove_topic - Remove an existing topic\n"
              << "  select_topic - Select a topic for questions\n"
              << "  list_topics - List all topics\n"
              << "  add_question - Add a new question to the selected topic\n"
              << "  remove_question - Remove a question from the selected topic\n"
              << "  generate_quiz - Generate a quiz based on the selected topic\n"
              << "  list_questions - List all questions in the selected topic\n";
}
void CLI::addTopic() {
    std::string topicName;
    std::cout << "Enter topic name: ";
    std::getline(std::cin, topicName);
    
    if (topicName.empty()) {
        std::cout << "Topic name cannot be empty.\n";
        return;
    }

    for (const auto& topic : db->topics) {
        if (topic->name == topicName) {
            std::cout << "Topic '" << topicName << "' already exists.\n";
            return;
        }
    }
    auto newTopic = std::make_shared<Topic>(topicName);
    db->topics.push_back(newTopic);
    
    std::cout << "Topic '" << topicName << "' added successfully.\n";
}
void CLI::removeTopic() {
    std::string topicName;
    std::cout << "Enter topic name to remove: ";
    std::getline(std::cin, topicName);
    
    auto it = std::remove_if(db->topics.begin(), db->topics.end(),
                             [&topicName](const std::shared_ptr<Topic>& topic) {
                                 return topic->name == topicName;
                             });
    
    if (it != db->topics.end()) {
        db->topics.erase(it, db->topics.end());
        std::cout << "Topic '" << topicName << "' removed successfully.\n";
    } else {
        std::cout << "Topic '" << topicName << "' not found.\n";
    }
}
void CLI::selectTopic(){
    int topicIndex;
    std::cout << "Enter topic index to select (0 to " << db->topics.size() - 1 << "): ";
    std::cin >> topicIndex;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (topicIndex < 0 || topicIndex >= static_cast<int>(db->topics.size())) {
        std::cout << "Invalid topic index.\n";
        return;
    }
    selectedTopicIndex = topicIndex;
    std::cout << "Selected topic: " << db->topics[selectedTopicIndex]->name << "\n";
}
void CLI::listTopics(){
    if (db->topics.empty()) {
        std::cout << "No topics available.\n";
        return;
    }
    std::cout << "Available topics:\n";
    for (size_t i = 0; i < db->topics.size(); ++i) {
        std::cout << i << ": " << db->topics[i]->name << "\n";
    }
}
void CLI::addQuestion(){
    if (selectedTopicIndex < 0 || selectedTopicIndex >= static_cast<int>(db->topics.size())) {
        std::cout << "No topic selected. Please select a topic first.\n";
        return;
    }

    std::string questionText;
    int questionType;
    std::optional<std::vector<std::string>> options;
    int correctOptionIndex;

    std::cout << "Enter question text: ";
    std::getline(std::cin, questionText);
    
    std::cout << "Enter question type (0 for single choice, 1 for multiple choice): ";
    std::cin >> questionType;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (questionType == 1) {
        int optionCount;
        std::cout << "Enter number of options: ";
        std::cin >> optionCount;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        options = std::vector<std::string>(optionCount);
        for (int i = 0; i < optionCount; ++i) {
            std::cout << "Enter option " << (i + 1) << ": ";
            std::getline(std::cin, options->at(i));
        }
        
        std::cout << "Enter index of the correct option (0 to " << optionCount - 1 << "): ";
        std::cin >> correctOptionIndex;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        if (correctOptionIndex < 0 || correctOptionIndex >= optionCount) {
            std::cout << "Invalid correct option index.\n";
            return;
        }
    } else {
        options = std::nullopt; 
        correctOptionIndex = -1; 
    }

    auto newQuestion = std::make_shared<Question>(questionText, questionType, options, correctOptionIndex, db->topics[selectedTopicIndex]);
    db->addQuestion(newQuestion);
    
    std::cout << "Question added successfully.\n";
}
void CLI::removeQuestion(){
    int questionIndex;
    std::cout << "Enter question index to remove (0 to " << db->getQuestionCount() - 1 << "): ";
    std::cin >> questionIndex;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (questionIndex < 0 || questionIndex >= db->getQuestionCount()) {
        std::cout << "Invalid question index.\n";
        return;
    }
    auto question = db->getQuestionByIndex(questionIndex);
    if (question->topic->name != db->topics[selectedTopicIndex]->name) {
        std::cout << "Question does not belong to the selected topic.\n";
        return;
    }
    db->removeQuestion(question);
    std::cout << "Question removed successfully.\n";
}
void CLI::generateQuiz(){
    auto topics = db->topics;
    if (topics.empty()) {
        std::cout << "No topics available to generate a quiz.\n";
        return;
    }
    for (size_t i = 0; i < topics.size(); ++i) {
        std::cout << i << ": " << topics[i]->name << "\n";
    }
    std::vector<std::pair<std::shared_ptr<Topic>, int>> selectedTopics;
    std::string input;
    std::cout << "Enter topic indices and number of questions per topic (e.g., '0 5' for topic 0 with 5 questions, or 'done' to finish): ";
    while (true) {
        std::getline(std::cin, input);
        if (input == "done") {
            break;
        }
        std::istringstream iss(input);
        int topicIndex, questionCount;
        if (!(iss >> topicIndex >> questionCount) || topicIndex < 0 || topicIndex >= static_cast<int>(topics.size()) || questionCount <= 0) {
            std::cout << "Invalid input. Please enter valid topic index and question count.\n";
            continue;
        }
        auto it = std::find_if(selectedTopics.begin(), selectedTopics.end(),
                               [topicIndex, &topics](const std::pair<std::shared_ptr<Topic>, int>& pair) {
                                   return pair.first->name == topics[topicIndex]->name;
                               });
        if (it != selectedTopics.end()) {
            it->second += questionCount; 
            continue;
        }
        auto questions = db->getQuestionsByTopic(topics[topicIndex]);
        if (questionCount > static_cast<int>(questions.size())) {
            std::cout << "Warning: Requested question count exceeds available questions in topic '" << topics[topicIndex]->name << "'. Setting to " << questions.size() << ".\n";
            questionCount = static_cast<int>(questions.size());
        }
        selectedTopics.emplace_back(topics[topicIndex], questionCount);
    }
    int variantCount;
    std::cout << "Enter the number of quiz variants to generate: ";
    std::cin >> variantCount;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (variantCount <= 0) {
        std::cout << "Invalid variant count. Must be greater than 0.\n";
        return;
    }
    std::cout << "Shuffle questions for each variant? (yes/no): ";
    std::string shuffleInput;
    std::getline(std::cin, shuffleInput);
    bool shuffleQuestions = (shuffleInput == "yes" || shuffleInput == "y");
    for (int i = 0; i < variantCount; ++i) {
        std::cout << "Generating variant " << (i + 1) << ":\n";
        auto quizVariant = std::make_shared<QuizVariant>("Вариант " + std::to_string(i + 1));
        for (const auto& topicPair : selectedTopics) {
            auto topic = topicPair.first;
            int questionCount = topicPair.second;
            auto questions = db->getQuestionsByTopic(topic);
            if (questions.size() < static_cast<size_t>(questionCount)) {
                std::cout << "Warning: Not enough questions in topic '" << topic->name << "' to fulfill the request. Adding all available questions.\n";
                questionCount = static_cast<int>(questions.size());
            }
            std::shuffle(questions.begin(), questions.end(), std::mt19937(std::random_device{}())); 
            for (int j = 0; j < questionCount; ++j) {
                quizVariant->addQuestion(questions[j]); 
            }
        }
        if (shuffleQuestions) {
            quizVariant->shuffleQuestions(); 
        }
        std::cout << "Quiz Variant '" << quizVariant->variantName << "' generated with " << quizVariant->getQuestions().size() << " questions:\n";
        for (const auto& question : quizVariant->getQuestions()) {
            std::cout << "- " << question->questionText << "\n";
            if (question->options) {
                std::cout << "  Options:\n";
                for (size_t j = 0; j < question->options->size(); ++j) {
                    std::cout << "  " << j << ": " << question->options->at(j) << "\n";
                }
                std::cout << "  Correct Option Index: " << question->correctOptionIndex << "\n";
            }
            std::cout << "  Topic: " << question->topic->name << "\n";
        }
        std::cout << "Variant " << (i + 1) << " generated successfully.\n";
        std::string fileName = "quiz_variant_" + std::to_string(i + 1) + ".html";
        try {
            db->writeExamToDoc(fileName, quizVariant);
            std::cout << "Quiz variant saved to " << fileName << ".\n";
        } catch (const std::exception& e) {
            std::cout << "Error saving quiz variant: " << e.what() << "\n";
        }
    }
    std::cout << "All quiz variants generated successfully.\n";
}
void CLI::listQuestions(){
    if (selectedTopicIndex < 0 || selectedTopicIndex >= static_cast<int>(db->topics.size())) {
        std::cout << "No topic selected. Please select a topic first.\n";
        return;
    }

    auto questions = db->getQuestionsByTopic(db->topics[selectedTopicIndex]);
    if (questions.empty()) {
        std::cout << "No questions available for the selected topic.\n";
        return;
    }

    std::cout << "Questions in topic '" << db->topics[selectedTopicIndex]->name << "':\n";
    for (size_t i = 0; i < questions.size(); ++i) {
        std::cout << i << ": " << questions[i]->questionText << "\n";
    }
}

void CLI::exit() {
    std::cout << "Exiting the application.\n";
    db->writeQuestionsToFile("db.txt");
    std::exit(0);
}