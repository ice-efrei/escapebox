#include <LiquidCrystal.h>
#include <Keypad.h>

// Keypad
const int ROW_NUM = 4;    // four rows
const int COLUMN_NUM = 4; // four columns

char keys[ROW_NUM][COLUMN_NUM] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

byte pin_rows[ROW_NUM] = {9, 8, 7, 6};      // connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {5, 4, 3, 2}; // connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

char listenKeypad()
{
    char key = keypad.waitForKey();
    if (key)
    {
        Serial.println(key);
    }
    return key;
}
int getFromTo(int min, int max)
{
    int value = -1;
    do
    {
        char key = listenKeypad();
        if (key)
        {
            value = key - '0';
        }
    } while (value < min || value > max);
    return value;
}
int getNumberOrStar()
{
    int value = -1;
    do
    {
        char key = listenKeypad();
        if (key)
        {
            value = key - '0';
        }
    } while (value < 0 || value > 9 || value == -6);
    return value;
}
char getChar()
{
    char value = '\0';
    do
    {
        char key = listenKeypad();
        if (key)
        {
            value = key;
        }
    } while (value == '\0');
    return value;
}
char getKeyIfPressed()
{
    char key = keypad.getKey();
    if (key)
    {
        Serial.println(key);
    }
    return key;
}

// LCD Pins
#define LCD_RS 8
#define LCD_EN 9
#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7

// LCD Dimensions
#define LCD_COLS 32
#define LCD_ROWS 4

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

void printCentered(String text, int row)
{
    Serial.println(text);
    int len = text.length();
    int pos = (LCD_COLS - len) / 2;
    lcd.setCursor(pos, row);
    lcd.print(text);
}

void printAt(String text, int row, int col)
{
    Serial.println(text);
    lcd.setCursor(col, row);
    lcd.print(text);
}

void printTwoColumns(String text1, String text2, int row)
{
    printAt(text1, row, 0);
    printAt(text2, row, LCD_COLS / 2);
}

void cleanLCD()
{
    lcd.clear();
}

void loadingBarWithTitle(String title)
{
    printCentered(title, 1);
    for (int i = 2; i < LCD_COLS - 2; i++)
    {
        lcd.setCursor(i, 2);
        lcd.print("=");
        delay(100);
    }
}

// EscapeBox
#define CODE_SIZE 6

String millisToString(unsigned long millis)
{
    int milliseconds = millis % 1000;
    int seconds = millis / 1000;
    int minutes = seconds / 60;
    int hours = minutes / 60;

    String time = "";
    if (hours < 10)
    {
        time += "0";
    }
    time += String(hours) + ":";
    if (minutes < 10)
    {
        time += "0";
    }
    time += String(minutes) + ":";
    if (seconds < 10)
    {
        time += "0";
    }
    time += String(seconds);
    if (milliseconds < 100)
    {
        time += "0";
    }
    if (milliseconds < 10)
    {
        time += "0";
    }
    time += String(milliseconds);

    return time;
}

// EscapeBox gameplays
const int gameplaysCount = 1;
String gameplays[] = {"Numpad"};

// EscapeBoxMachine class
class EscapeBoxMachine
{
    /**
     * _state:
     * 0 : credits
     * 1 : main menu
     * 2 : gameplay initialization
     * 3 : game started
     */
public:
    void printOnLCD(String text, int row)
    {
        printCentered(text, row);
    }

    void initialize()
    {
        loadState(0);
        _timer = 0;
    }

    void update()
    {
        switch (_state)
        {
        case 0:
            displayCredits();
            break;
        case 1:
            displayMainMenu();
            break;
        case 2:
            initGameplays();
            break;
        case 3:
            updateGameplay();
            break;
        }
    }

private:
    int _state;
    int _gameplayIndex;

    int _timer; // time in milliseconds until the bomb explodes
    char _defuse_code[CODE_SIZE];

    unsigned long _previous_time;

    int _code_current_index = 0;
    char _defusing_code[CODE_SIZE];

    void loadState(int state)
    {
        _state = state;
        loadingBarWithTitle("Loading...");
    }
    void initGameplays()
    {
        initTimer();

        switch (_gameplayIndex)
        {
        case 0:
            initNumpadGameplay();
            break;
        case 1:
            break;
        }

        loadState(3);
    }
    void initTimer()
    {
        int time[4] = {0, 0, 0, 0}; // HH:MM

        int i = 0;
        while (i < 4)
        {
            cleanLCD();
            printCentered("Set time", 1);
            printCentered(String(time[0]) + String(time[1]) + ":" + String(time[2]) + String(time[3]), 2);
            time[i] = getNumberOrStar();
            if (time[i] < 0)
            {
                time[i] = 0;
                if (i > 0)
                {
                    i--;
                    time[i] = 0;
                }
            }
            else
            {
                i++;
            }
        }

        _timer = ((time[0] * 10 + time[1]) * 60 + (time[2] * 10 + time[3])) * 1000;
    }
    void updateGameplay()
    {
        unsigned long current_time = millis();

        if (current_time - _previous_time < 10)
        {
            _timer -= current_time - _previous_time;
        }

        switch (_gameplayIndex)
        {
        case 0:
            updateNumpadGameplay();
            break;
        default:
            break;
        }

        _previous_time = current_time;
    }

    /**
     * Display the credits
     * State 0
     */
    void displayCredits()
    {
        printCentered("EscapeBox v1", 1);
        printCentered("An ICE EFREI project", 2);

        delay(2000);
        cleanLCD();

        loadState(1);
    }

    /**
     * Display the main menu
     * State 1
     */
    void displayMainMenu()
    {
        printCentered("Select gameplay", 0);
        // display the gameplays by columns of two with index number 1:
        // 1. lorem 2. ipsum
        // 3. dolor 4. sit
        // 5. amet 6. ...
        int lineCounter = 1;
        for (int i = 0; i < gameplaysCount; i++)
        {
            if (i % 2 == 0)
            {
                if (i + 1 < gameplaysCount)
                {
                    printTwoColumns(String(i + 1) + ". " + gameplays[i], String(i + 2) + ". " + gameplays[i + 1], lineCounter);
                }
                else
                {
                    printAt(String(i + 1) + ". " + gameplays[i], i / 2, lineCounter);
                }
                lineCounter++;
            }
        }

        delay(2000);
        cleanLCD();

        do
        {
            _gameplayIndex = getFromTo(1, gameplaysCount) - 1;
        } while (_gameplayIndex == -1);

        loadState(2);
    }

    /**
     * Gameplay: Numpad
     * State 2
     * Gameplay index: 0
     */
    void initNumpadGameplay()
    {
        if (_timer == 0)
            return;

        for (int i = 0; i < CODE_SIZE; i++)
        {
            _defuse_code[i] = '*';
            _defusing_code[i] = '*';
        }

        printCentered("Defuse the bomb", 0);
        printCentered("Enter the code", 1);

        int i = 0;
        while (i < 6)
        {
            printCentered(String(_defuse_code[0]) + String(_defuse_code[1]) + String(_defuse_code[2]) + String(_defuse_code[3]) + String(_defuse_code[4]) + String(_defuse_code[5]), 2);
            _defuse_code[i] = getChar();
            if (_defuse_code[i] == '*')
            {
                _defuse_code[i] = '*';
                if (i > 0)
                {
                    i--;
                    _defuse_code[i] = '*';
                }
            }
            else
            {
                i++;
            }
        }

        printCentered("Code entered", 3);
        delay(2000);
        cleanLCD();
    }

    /**
     * Gameplay : Numpad
     * State : 3
     * Gameplay index : 0
     */
    void updateNumpadGameplay()
    {
        if (_timer == 0)
            return;

        if (_timer <= 0)
        {
            loadState(4);
            return;
        }

        String timeLeft = millisToString(_timer);
        printTwoColumns("Time left", timeLeft, 0);

        printCentered(String(_defuse_code[0]) + String(_defuse_code[1]) + String(_defuse_code[2]) + String(_defuse_code[3]) + String(_defuse_code[4]) + String(_defuse_code[5]), 2);

        const char pressed = getKeyIfPressed();

        if (pressed != '\0')
        {
            for (int i = 0; i < CODE_SIZE - 1; i++)
            {
                _defusing_code[i] = _defusing_code[i + 1];
            }
            _defusing_code[CODE_SIZE - 1] = pressed;
            _code_current_index++;

            if (String(_defusing_code) == String(_defuse_code))
            {
                loadState(5);
                return;
            }
            else if (_code_current_index >= CODE_SIZE)
            {
                for (int i = 0; i < CODE_SIZE; i++)
                {
                    _defusing_code[i] = '*';
                }
                _code_current_index = 0;
            }
        }
    }

    /**
     * Bomb Explode
     * State 4
     */
    void gameover()
    {
        printCentered("BOOM", 1);
        printCentered("EXPLODE", 2);
        printCentered("press any key", 3);

        listenKeypad();

        cleanLCD();

        loadState(1);
    }

    /**
     * Bomb Defused
     * State 5
     */
    void defused()
    {
        printCentered("Bomb defused", 1);

        printCentered("press any key", 3);

        listenKeypad();

        cleanLCD();

        loadState(1);
    }
};

EscapeBoxMachine *ebm;

void setup()
{
    Serial.begin(9600);

    lcd.begin(LCD_COLS, LCD_ROWS);
    printAt("Hello World!", 0, 0);

    ebm = new EscapeBoxMachine();
    ebm->initialize();
}

void loop()
{
    ebm->update();
}
