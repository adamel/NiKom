 #include <string.h>
#include <time.h>
#include <stdlib.h>

#include <exec/types.h>
#include <utility/tagitem.h>

#include "/include/nikomstr.h"
#include "/Include/NiKomLib.h"
#include "funcs.h"
#include "nikombase.h"

char *getcryptkey(void);

/****** nikom.library/CheckPassword ******************************************

    NAME
        CheckPassword -- Kollar en anv�ndares l�senord.

    SYNOPSIS
        error = CheckPassword ( usernumber, password)
        d0                   	    d0		   a0
        int CheckPassword ( LONG, char *)

    FUNCTION
    	Kollar om l�senordet som anges st�mmer med anv�ndarens
    	l�senord.
    	
    INPUTS
    	usernumber - Anv�ndarnumret.
        password   - L�senord att j�mf�ra med anv�ndarens l�senord.

    RESULT
        error - Ger f�ljande felmeddeladen:
             1	- L�senordet var korrekt.
             0	- L�senordet var inte korrekt.
            -1  - Finns ingen s�dan anv�ndare

        
    EXAMPLE

    NOTES

    BUGS

    SEE ALSO
        DeleteUser(), EditUser(), ReadUser() och SortUserlist()

******************************************************************************
* F�ljande b�r nog nollas: US_Group, US_Brevpek, US_Textpek och resten...
*
*/

int __saveds __asm LIBCheckPassword(register __d0 LONG usernumber, register __a0 char *password, register __a6 struct NiKomBase *NiKomBase)
{
	char CurrentPassword[16];
	char key[KEYLENGTH+1];
	int i;

	if(ReadUser(usernumber, US_Name) == (void *)-2)
		return -1;

	strcpy(CurrentPassword, (char *)ReadUser(usernumber, US_Password));

	if(NiKomBase->Servermem->cfg.cfgflags & NICFG_CRYPTEDPASSWORDS)
	{ /* Krypterat l�senord */
		for(i=0;i<KEYLENGTH;i++)
			key[i] = CurrentPassword[i];
		key[KEYLENGTH] = 0;

		if(!(strcmp(crypt(password, key), CurrentPassword)))
			return 1;
		else
			return 0;
	}
	else
	{ /* Icke krypterat l�senord */
		if(!(strcmp(CurrentPassword, password)))
			return 1;
		else
			return 0;
	}
}

char *__saveds __asm LIBCryptPassword(register __a0 char *password, register __a6 struct NiKomBase *NiKomBase)
{
	char crypted[36], key[KEYLENGTH+1];
	
	strcpy(key, getcryptkey());
	strcpy(crypted, crypt(password, key));
	return &crypted[0];
}

char *getcryptkey(void)
{
	int i, slump;
	char key[KEYLENGTH+1];
	unsigned int clock[2];

	timer(clock);
	srand(clock[1]);

	for(i=0;i<KEYLENGTH+1;i++)
	{
		slump = rand() % 50;
		if(slump<25)
		{
			key[i] = slump + 65;
		}
		else
		{
			key[i] = slump + 72;
		}
	}
	key[KEYLENGTH] = NULL;

	return &key[0];
}
