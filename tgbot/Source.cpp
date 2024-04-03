#include <stdio.h>
#include <tgbot/tgbot.h>

#define SQLITECPP_COMPILE_DLL
#include <SQLiteCpp/SQLiteCpp.h>

using namespace std;
int x = 0;

//создаем/открываем базу данных 
SQLite::Database db("data_base_for_tgbot.db3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

void updateXByIdTg(long long int idTg, int x)
{
    try
    {
        SQLite::Statement query(db, "UPDATE users SET x = :x WHERE idTg = :idTg");
        query.bind(":idTg", idTg);
        query.bind(":x", x);

        cout << query.getExpandedSQL() << endl;
        query.exec();
    }
    catch (exception& e) 
    {
        cerr << e.what() << endl;
    }
}

int main() {
    
    TgBot::Bot bot("6735653705:AAHWqAaSw0KZYX0GYox_5ErhkN5qUB1haLc");

    srand(time(0));

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi! I entered the number from 1 to 50. Guess him!");
        //bot.getApi().sendMessage(message->chat->id, "But first enter your name:");
        x = rand() % 50 + 1;
        cout << x << endl;

        try {
            SQLite::Statement query(db, "SELECT COUNT(*) FROM users WHERE idTg = :userId");
            query.bind(":userId", message->chat->id); //привязка значения к параметру :userId

            if (query.executeStep())
            {
                int count = query.getColumn(0).getInt();
                if (count > 0)
                {
                    cout << "ID " << message->chat->id << "found" << endl;
                    updateXByIdTg(message->chat->id, x);
                }
                else
                {
                    cout << "ID " << message->chat->id << "not found" << endl;
                    //добавляем данные
                    SQLite::Statement query(db, "INSERT INTO users (idTg) VALUES(?)");
                    query.bind(1, message->chat->id);
                    query.exec();

                    SQLite::Statement query(db, "INSERT INTO users (x) VALUES(?)");
                    query.bind(1, x);
                    query.exec();
                }
            }
        }
        catch (std::exception& e)
        {
            cerr << e.what() << endl;
        }

    });

    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {

        cout << message->chat->id << endl;
        printf("User wrote %s\n", message->text.c_str()); 

        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }
        
        int answer = atoi(message->text.c_str()); 
        if (answer < 1 || answer > 50)
        {
            bot.getApi().sendMessage(message->chat->id, "Are you stupid? I said from 1 to 50...");
            return;
        }
        try {
            SQLite::Statement query(db, "SELECT x FROM users WHERE idTg = :userId");
            query.bind(":userId", message->chat->id); //привязка значения к параметру :userId

            if (query.executeStep())
            {
                int x = query.getColumn(0).getInt();
                if (x > 0)
                {
                    if (answer > x)
                    {
                        bot.getApi().sendMessage(message->chat->id, "A lot of");
                    }
                    if (answer < x)
                    {
                        bot.getApi().sendMessage(message->chat->id, "Few");
                    }
                    if (answer == x)
                    {
                        bot.getApi().sendMessage(message->chat->id, "You guessed it!");
                    }
                    
                }
                else
                {
                    bot.getApi().sendMessage(message->chat->id, "ERROR!");
                    cout << "ERROR" << endl;
                }
            }
        }
        catch (std::exception& e)
        {
            cerr << e.what() << endl;
        }

        

    });

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    }
    catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }
    return 0;
}