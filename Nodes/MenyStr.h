
struct keywordlist {
        char    key;                    // tangent
        char   *name;                   // pekare till str�ng, inneh�llandes antingen menynamn eller
                                        // funktionsnummer
        char    arg;                    // Y= argument, N=inget argument till�ts.
        char   *argument;               // Ett defaultargument.
        char   *visastring;             // Det som visas f�r anv�ndaren d� ett argument skall s�ttas
        struct keywordlist *next;       // Pekare till n�sta keyword, NULL om sista
};



struct Meny {
        char *name;             // Menynamn
        int     status;         // Vilken status man m�ste ha f�r att se denna menydefinition
        int     ret;            // status f�r vad som g�ller vid endast return-tryckning
        void **menytext;        // pekare till array med menytextrader
        struct keywordlist *key;// pekare till lista med keywords
        struct Meny *next;      // pekare till n�sta meny
        BOOL visaarea;
        BOOL visamote;
        char showmenukey;       //Tangent f�r att visa menyn.
        struct keywordlist *startkom;  // Kommando som skall k�ras vid start av denna meny.
};

struct UserCfg {
        char *name;             //pekare till namnet p� den menydefinitionsfil som anv�ndaren valt
        BOOL hotkeys;           //TRUE=Hotkeys, FALSE=m�jlighetatt stacka flera komamndon p� en rad
        BOOL expert;            //TRUE=expertmode ON, FALSE=expertmode off
        char *sprak;            //pekare till namnet p� den spr�kfil som anv�ndaren valt
        BOOL scrclr;            //TRUE=rensa skr�men d� och d�.
};

struct enkellista {
        char *name;
        struct enkellista *next;
};


/*
0  -  Black              Dark Grey            Black           ESC [ 30 m
1  -  Red                Bright Red           Bright Red      ESC [ 31 m
2  -  Green              Bright Green         Bright Green    ESC [ 32 m
3  -  Orange             Yellow               Yellow          ESC [ 33 m
4  -  Dark Blue          Bright Blue          Dark Blue       ESC [ 34 m
5  -  Violet             Bright Violet        Violet          ESC [ 35 m
6  -  Cyan               Bright Cyan          Medium Blue     ESC [ 36 m
7  -  Light Grey         White                White           ESC [ 37 m        DEFAULT
*/

#define BLACK           "\x1b[30m"
#define RED             "\x1b[31m"
#define GREEN           "\x1b[32m"
#define ORANGE          "\x1b[33m"
#define DARK_BLUE       "\x1b[34m"
#define VIOLET          "\x1b[35m"
#define CYAN            "\x1b[36m"
#define LIGHT_GREY      "\x1b[37m"
#define DEFAULT         "\x1b[0m"


/*
* S= Statusniv�
* L= Minsta antal inloggningar f�r att utf�ra kommandot
* X= Ett eventuellt l�senord f�r att f� utf�ra komamndot
*/

struct menykommando
{
        int nummer;             // nummret p� kommandot
        int status;             // MInimal status
        int minlogg;             // minst antal inloggningar
        int mindays;            // minst antal dagar
        char losen[20];         // l�sen
        char vilkainfo[50];     // Vad som syns vid VILKA -V
        char logstr[50];        // vad som skrivs till loggfilen
        long before;
        long after;
        long grupper;
        struct menykommando *next;
};

