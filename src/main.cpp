#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QDateTime>
#include <QDialog>
#include <QInputDialog>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QStack>
#include <QList>
#include <QTimer>
#include <QTextEdit>
#include <QObject>
#include <QUrl>
#include <nlohmann/json.hpp>
#include <string>
#include <curl/curl.h>
#include <chrono>
#include <ctime>
#include <iostream>
using namespace std;
using namespace nlohmann;

class Message {
public:
    QString text;
    QString sender;
    QDateTime timestamp;
};

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char *)contents, newLength);
    } catch (std::bad_alloc &e) {
        // Обработка ошибок при нехватке памяти
        return 0;
    }
    return newLength;
}

pair<std::string, std::string> take_cords(string address){
    CURL *curl;
    CURLcode res;
    string readBuffer;
    stringstream lat_ss;
    stringstream lon_ss;
    float lat_f;
    float lon_f;
    string lat;
    string lon;
    string url;
    curl = curl_easy_init();
    if(curl){
        //"https://catalog.api.2gis.com/3.0/items/geocode?q=Москва, Садовническая, 25&fields=items.point&key=499ee3fd-1a17-4c8f-be92-5263b3470dc8
        url = "https://catalog.api.2gis.com/3.0/items/geocode?q=" + address + "&fields=items.point&key=499ee3fd-1a17-4c8f-be92-5263b3470dc8";
        /* string url = "https://geocode-maps.yandex.ru/1.x/?apikey=dbcd5d6e-f034-4c9e-a84d-11c201dbd4d4&lang=ru_RU&geocode="+ city +"&format=json"; определение station города*/
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        if (res == CURLE_OK){
            json response = json::parse(readBuffer);
            lat_f = response["result"]["items"][0]["point"]["lat"];
            lon_f = response["result"]["items"][0]["point"]["lon"];
            lat_ss << lat_f;
            lon_ss << lon_f;
            lat  = lat_ss.str();
            lon = lon_ss.str();
            /*pos = pos.replace(pos.find(" "),1,"&lat=");*/                                                                                                                        //37.617698&lan=55.755864
            
        }
        else {
            std::cerr << "Ошибка при выполнении запроса: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return make_pair(lat, lon);
}

json getFiveDayWeatherForecast(const std::string &apiKey, string lat , string lon) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    json response;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "http://api.openweathermap.org/data/2.5/forecast?&lat="+lat+"&lon="+lon+"&appid=" + apiKey;
        cout << endl << url << endl;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            response = json::parse(readBuffer);
            readBuffer.clear();
        } else {
            std::cerr << "Ошибка при выполнении запроса: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    return response;
}

void setWindowSize(QMainWindow& window, int width, int height) {
    window.resize(width, height);
}

void setHistoryWindowSize(QDialog& dialog, int width, int height) { dialog.resize(width, height); }

void addMessageToHistory(QList<Message>& messageHistory, const QString& sender, const QString& text) {
    Message newMessage;
    newMessage.text = text;
    newMessage.sender = sender;
    newMessage.timestamp = QDateTime::currentDateTime();
    messageHistory.append(newMessage);
}

void displayMessage(QTextEdit* chatBox, QTextEdit* chatBox_2, const QString& sender, const QString& text) {
    if (sender == "Bot") {
        chatBox->append(sender + ": " + text);
    } 
    else if(text == "помощь"){
        chatBox_2->append(sender + ": " + text + "\n"+ "\n"+ "\n"+ "\n"+ "\n"+ "\n"+ "\n"+ "\n");
    }
    else {
        chatBox_2->append(sender + ": " + text + "\n");
    }
}


bool isOperator(const QChar &op) {
    return op == '+' || op == '-' || op == '*' || op == '/';
}

double evaluateExpression(const QString &expression) {
    QStack<double> operands;
    QStack<QChar> operators;

    for (int i = 0; i < expression.length(); i++) {
        if (expression[i].isDigit() || expression[i] == '.') {
            QString number;
            while (i < expression.length() && (expression[i].isDigit() || expression[i] == '.')) {
                number.append(expression[i]);
                i++;
            }
            operands.push(number.toDouble());
            i--;
        } else if (isOperator(expression[i])) {
            while (!operators.isEmpty() && ((expression[i] == '+' || expression[i] == '-') &&
                    (operators.top() == '*' || operators.top() == '/'))) {
                double operand2 = operands.pop();
                double operand1 = operands.pop();
                QChar op = operators.pop();
                double result;
                if (op == '+') {
                    result = operand1 + operand2;
                } else if (op == '-') {
                    result = operand1 - operand2;
                } else if (op == '*') {
                    result = operand1 * operand2;
                } else {
                    result = operand1 / operand2;
                }
                operands.push(result);
            }
            operators.push(expression[i]);
        } else if (expression[i] == '(') {
            operators.push(expression[i]);
        } else if (expression[i] == ')') {
            while (operators.top() != '(') {
                double operand2 = operands.pop();
                double operand1 = operands.pop();
                QChar op = operators.pop();
                double result;
                if (op == '+') {
                    result = operand1 + operand2;
                } else if (op == '-') {
                    result = operand1 - operand2;
                } else if (op == '*') {
                    result = operand1 * operand2;
                } else {
                    result = operand1 / operand2;
                }
                operands.push(result);
            }
            operators.pop();
        }
    }

    while (!operators.isEmpty()) {
        double operand2 = operands.pop();
        double operand1 = operands.pop();
        QChar op = operators.pop();
        double result;
        if (op == '+') {
            result = operand1 + operand2;
        } else if (op == '-') {
            result = operand1 - operand2;
        } else if (op == '*') {
            result = operand1 * operand2;
        } else {
            result = operand1 / operand2;
        }
        operands.push(result);
    }

    return operands.pop();
}

void getTime(QTextEdit* chatBox) {
    QDateTime currentTime = QDateTime::currentDateTime();
    chatBox->append("\nBot: Текущее время: " + currentTime.toString("hh:mm:ss"));
}

void respondToMessage(QTextEdit* chatBox, const QString& messageText, QList<Message>& messageHistory) {
    std::string apiKey = "1ff15930de8f02537dc1d994d0ad603e";
    if (messageText.toLower() == "привет" ) {
        chatBox->append("\nBot: Привет!");
        addMessageToHistory(messageHistory, "Bot", "Привет!");
    } else if (messageText.toLower() == "что такое боль") {
        chatBox->append("\nBot: Боль в шее, пока ты писал меня.");
        addMessageToHistory(messageHistory, "Bot", "Не знаю)");
    }else if (messageText.toLower() == "что такое радость") {
        chatBox->append("\nBot: Залитый я на git");
        addMessageToHistory(messageHistory, "Bot", "Залитый я на git");
    } else if (messageText.toLower() == "время") {
       getTime(chatBox);
       addMessageToHistory(messageHistory, "Bot", "Текущее время: " + QDateTime::currentDateTime().toString("hh:mm:ss"));
    }else if (messageText.contains(QRegularExpression("[A-ZА-Я]"))) { // messageText.contains(QRegularExpression("[0-9\\+\\-\\*\\/\\(\\)]+"))
        string message = messageText.toStdString(); // moscov
        pair<string, string> result = take_cords(message);
        string lat = result.first;
        string lon = result.second;
        json fiveDayForecast = getFiveDayWeatherForecast(apiKey, lat,lon);
        int i = 0;
        std::string weather;
        weather = std::to_string(fiveDayForecast["list"][0]["main"]["temp"].get<double>())+" "+fiveDayForecast["list"][0]["weather"][0]["description"].get<std::string>();
        QString Qweather = QString::fromStdString(weather);
        chatBox->append("\nBot: Погода " + Qweather);
        addMessageToHistory(messageHistory, "Bot", "Погода " + Qweather);
    } else if (messageText.contains(QRegularExpression("[0-9\\+\\-\\*\\/\\(\\)]+"))) {
        double result = evaluateExpression(messageText);
        chatBox->append("\nBot: Результат: " + QString::number(result));
        addMessageToHistory(messageHistory, "Bot", "Результат: " + QString::number(result));
    }
    else if (messageText.toLower() == "помощь") {
        chatBox->append("\nBot: Доступные команды:");
        chatBox->append("1. привет - поприветствовать бота");
        chatBox->append("2. что такое боль - ответ");
        chatBox->append("3. что такое радость - ответ");
        chatBox->append("4. время - получить текущее время");
        chatBox->append("5. примерчик - вычислить результат выражения");
        chatBox->append("6. пока - закрытие программы");
        chatBox->append("7. Город - погода в городе");
        addMessageToHistory(messageHistory, "Bot", "Доступные команды:\n1. привет \n2. что такое боль \n3. что такое радость \n4. Время \n5. примерчик \n 6. Город");
    } else if (messageText.toLower() == "пока") {
        chatBox->append("\nBofont-weight: bold;t: Пока");
        addMessageToHistory(messageHistory, "Bot", "Пока");
        QTimer::singleShot(1000, [](){ QApplication::exit(); });
    } else {
        chatBox->append("\nBot: Я не понимаю ваш запрос.");
        addMessageToHistory(messageHistory, "Bot", "Я не понимаю ваш запрос.");
    }
}


void updateHistoryBox(QTextEdit* historyBox, const QList<Message>& messageHistory) {
    historyBox->clear();
    for (const Message& message : messageHistory) {
        historyBox->append(message.timestamp.toString("dd.MM.yyyy hh:mm:ss") + " - " + message.sender + ": " + message.text);
    }
}

void saveMessageHistory(const QList<Message>& messageHistory, const QString& username) {
    QFile file(username + ".txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const Message& message : messageHistory) {
            out << message.timestamp.toString(Qt::ISODate) << "," << message.sender << "," << message.text << "\n";
        }
        file.close();
    }
}

QList<Message> loadMessageHistory(const QString& username) {
    QList<Message> messageHistory;
    QFile file(username + ".txt");

    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Не удалось создать файл истории сообщений:";
            return messageHistory;
        }
        file.close();

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Не удалось открыть файл истории сообщений для чтения:";
            return messageHistory;
        }
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QStringList parts = in.readLine().split(",");
        if (parts.size() == 3) {
            Message message;
            message.timestamp = QDateTime::fromString(parts[0], Qt::ISODate);
            message.sender = parts[1];
            message.text = parts[2];
            messageHistory.append(message);
        }
    }
    file.close();

    return messageHistory;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString username = QInputDialog::getText(nullptr, "Введите ваше имя", "Пожалуйста введите имя:");


    if(username.isEmpty()) {
        return 0;
    }

    QList<Message> messageHistory = loadMessageHistory(username);

    QObject::connect(&app, &QApplication::aboutToQuit, [&, username]() {
        saveMessageHistory(messageHistory, username);
    });

    QMainWindow window;
    window.setWindowTitle("ChatBot");
    setWindowSize(window, 1000, 800);
    window.setWindowIcon(QIcon("../src/style/ico/cyber.png"));

    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();
    QHBoxLayout *H_layout = new QHBoxLayout();
    QHBoxLayout *H_layout_2 = new QHBoxLayout();

    QTextEdit *chatBox = new QTextEdit();
    chatBox->setReadOnly(true);


    QTextEdit *chatBox_2 = new QTextEdit();
    chatBox_2->setReadOnly(true);
    QTextCursor cursor(chatBox_2->textCursor());
    QTextBlockFormat format;
    format.setAlignment(Qt::AlignRight);
    cursor.setBlockFormat(format);
    chatBox_2->setTextCursor(cursor);

    QLineEdit *inputBox = new QLineEdit();
    inputBox->setMinimumHeight(45);

    QPushButton *sendButton = new QPushButton("SEND");
    QPushButton *historyButton = new QPushButton("HISTORY");

    QVBoxLayout *historyLayout = new QVBoxLayout();
    QTextEdit *historyBox = new QTextEdit();
    historyBox->setReadOnly(true);

    H_layout->addWidget(sendButton);
    H_layout->addWidget(historyButton);

    H_layout_2->addWidget(chatBox);
    H_layout_2->addWidget(chatBox_2);

    layout->addLayout(H_layout_2);
    layout->addWidget(inputBox);
    layout->addLayout(H_layout);

    widget->setLayout(layout);

    QObject::connect(sendButton, &QPushButton::clicked, [chatBox, chatBox_2, inputBox, &messageHistory, historyBox, username]() {
        QString messageText = inputBox->text();
        inputBox->clear();

        addMessageToHistory(messageHistory, username, messageText);
        displayMessage(chatBox, chatBox_2, username, messageText);

        respondToMessage(chatBox, messageText, messageHistory);
    });

    QObject::connect(inputBox, &QLineEdit::returnPressed, [sendButton]() {
        sendButton->click();
    });

    QObject::connect(historyButton, &QPushButton::clicked, [historyBox, &messageHistory]() {
        QDialog historyDialog;
        setHistoryWindowSize(historyDialog, 400, 400);
        QVBoxLayout *historyDialogLayout = new QVBoxLayout();
        historyDialogLayout->addWidget(historyBox);
        historyDialog.setLayout(historyDialogLayout);

        updateHistoryBox(historyBox, messageHistory);

        historyDialog.exec();
    });

    QFile styleFile("../src/style/style_qss/design.qss");
    styleFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(styleFile.readAll());
    app.setStyleSheet(styleSheet);

    window.setCentralWidget(widget);
    window.show();

    return app.exec();
}
