/* Program f�r att installera nya ExtKomxxxx.nik

F�r att man ska kunna installera ett kommando med det h�r programmet kr�vs
det att det f�ljer standarden och har f�ljande f�lt i kommentaren i b�rjan
av filen:
 N=Namn p� kommandot
 O=Antal ord
 #=Nummer p� kommandot

Eventuellt kan det �ven st� vilken typ av argument kommandot tar. St�r
inget s�dant f�lt s� antas det att kommandot inte tar n�got argument. Se
Installation.doc f�r mer information om hur kommandon definieras i
Kommandon.cfg
*/

parse arg filnamn .

if ~open('fil',filnamn,'R') then do
	say 'Kan inte hitta n�gon fil med namnet' filnamn
	exit 10
end

namnfound=0
ordfound=0
nummerfound=0
argumentfound=0

do until eof('fil')
	inrad=readln('fil')
	select
		when left(inrad,2)='N=' then do
			namnfound=1
			namn=right(inrad,length(inrad)-2)
		end
		when left(inrad,2)='O=' then do
			ordfound=1
			ord=right(inrad,length(inrad)-2)
		end
		when left(inrad,2)='#=' then do
			nummerfound=1
			nummer=right(inrad,length(inrad)-2)
		end
		when left(inrad,2)='A=' then do
			argumentfound=1
			argument=right(inrad,length(inrad)-2)
		end
		when index(inrad,'*/')>0 then leave
		otherwise nop
	end
end
call close('fil')

if ~namnfound & ~ordfound & ~nummerfound then do
	say 'Inte tillr�ckligt med information i b�rjan av programmet.'
	say 'Kan inte installera det.'
	exit
end

say 'Vilken status ska beh�vas f�r att utf�ra kommandot?'
parse pull status
say
say 'Minsta antal dagar en anv�ndare en anv�ndare m�ste ha varit registrerad'
say 'f�r att f� utf�ra kommandot (0 f�r ingen begr�nsning)'
parse pull dagar
say
say 'Minsta antal inloggningar f�r att f� utf�ra kommandot'
say '(0 f�r ingen begr�nsning)'
parse pull inlogg
say
say 'Ev l�senord f�r att f� utf�ra kommandot'
parse pull losen

say
say
say 'Installera filen' filnamn
say 'som NiKom:Rexx/ExtKom'||nummer||'.nik'
say
say 'N='||namn
say 'O='||ord
say '#='||nummer
if argumentfound then say 'A='||argument
if dagar>0 then say 'D='||dagar
if inlogg>0 then say 'L='||inlogg
if losen~='' then say 'X='||losen
say
say '�r detta ok? (j/N)'
parse pull ok
if upper(ok)~='J' then exit

address command 'copy' filnamn 'NiKom:Rexx/ExtKom'||nummer||'.nik'
if exists(nummer||'.man') then address command 'copy' nummer||'.man NiKom:Lappar/'
if ~open('cfg','NiKom:DatoCfg/Kommandon.cfg','A') then do
	say 'Kunde inte �ppna Kommandon.cfg'
	exit 10
end

call writeln('cfg','')
call writeln('cfg','N='||namn)
call writeln('cfg','O='||ord)
call writeln('cfg','#='||nummer)
if argumentfound then call writeln('cfg','A='||argument)
if dagar>0 then call writeln('cfg','D='||dagar)
if inlogg>0 then call writeln('cfg','L='||inlogg)
if losen~='' then call writeln('cfg','X='||losen)
call close('cfg')

say
say 'L�sa in konfigurationsfilerna s� att kommandot kan anv�ndas? (j/N)'
parse pull ok
if upper(ok)~='J' then call ReadConfig()
