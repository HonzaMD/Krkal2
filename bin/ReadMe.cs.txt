========================================================================
KRKAL
========================================================================


O PRODUKTU
==========

   KRKAL je rozsáhlı systém pro vıvoj her, které se odehrávají na 
obdélníkovém plánu. V systému KRKAL lze vytváøet úplnì nové hry, 
vylepšovat hry ji hotové, vytváøet nové herní úrovnì (levely) 
a samozøejmì také všechny tyto hry hrát. 
   Souèástí je i znovu vytvoøená hra Krkal, vıkonnı grafickı engine,
editor levlù a skriptù, kompilátor. 
   Krkal byl vytvoøen jako projekt na Matematicko Fyzikální Fakultì
Univerzity Karlovy.

   Hra Krkal je logicko-akèní hra, kde si vyzkoušíte ovládání jedné 
ze tøí postavièek a pomùete jí vyøešit všechny nástrahy a pøekonat
rafinované pøekáky a nakonec se ve zdraví dostat do vytoueného
cíle kadého levelu - do exitu. 
   Levely jsou postaveny z mnoha rozmanitıch prvkù èi pøedmìtù, které 
ovlivòují jak sami sebe navzájem, tak i vás. Budete posouvat kameny, 
klouzat po ledu, vyhıbat se minám, správnì nastavovat rùzná tlaèítka, 
bloudit v bludištích a tøeba pøestavovat znaèky pro dopravní pøíšery...
   Hra Krkal je prostì velmi zajímavá a originální. Vše vidíte 
pøehlednì z vıšky a díky Systému Krkal nechybí kvalitní grafika
i hudba. V Systému je moné nejen editovat nové levly, ale pøímo
vytváøet zcela nové herní objekty, vylepšovat a modifikovat samotnou
hru!


LICENCE
=======

 # Program Krkal 2.? (demo) je volnì ke staení, mùete program dál 
   bezplatnì šíøit. 
 # Je moné program pouívat, modifikovat v nìm vytváøené hry, tvoøit 
   nové levly. 
 # Program pouíváte na vaše vlastní nebezpeèí. Nepøebíráme zodpovìdnost
   za jakékoli škody vzniklé v souvislosti s pouíváním programu. 
 # Bez našeho svolení není moné pouít Systém Krkal jako souèást 
   nìjakého komerènì prodávaného produktu. 
 # Veškerá další autorská práva vyhrazena (c) 2004


AUTOØI
======

#  Petr Altman (pal) pal@krkal.org
grafickı engine, editor textu, editor skriptù, grafika pro hru, www,... 

#  Jan Krèek (MD) md@krkal.org
kernel, skriptování, vlastní hra KRKAL, levely, ... 

#  Jan Poduška (Morgue)
GUI, editor levelù 

#  Jiøí Margaritov
kompilátor, interpret 

#  Jára Altmann
levely, nápady, návrhy, pøipomínky 

#  Jan Frolík
hudba - co dodat - VY - NI - KA - JÍ - CÍ 


INTERNETOVÉ STRÁNKY
===================

	www.krkal.org


POADAVKY NA SYSTÉM
===================

 # MS Windows XP, 2000, Me, 9x 
 # DirectX 8.1 nebo novìjší 
 # Procesor taktovanı kolem 1GHz nebo lepší 
 # nejménì 128 MB RAM 
 # 32MB DirectX kompatibilní grafická karta schopná pracovat  
   v 32bit hloubce (testováno na GeForce2 MX, GeForce 3, nForce, 
   Matrox Millenium G450) 
 # DirectX kompatibilní zvuková karta 
 # cca 30MB volného místa na HDD 


INSTALACE, KONFIGURACE A ZÁKLADNÍ INFORMACE
===========================================

 # Pøekopírujte adresáø s Krkalem na váš pevnı disk 
 # Spuste hru (systém) poklepáním na soubor krkal.exe 
 # Dokumentaci najdete v adresáøi doc nebo Dokumentace
 # Konfigurace se provádí ruèní editací souboru "krkal.cfg",
   je moné napøíklad mìnit rozlišení ve høe, nastavovat hlasitost,..
 # Odinstalace: Pouijte odinstalaèní program. Pokud ho nemáte, staèí 
   smazat adresáø s Krkalem, Krkal si nic do Windousù neukládá.
 # Na našich stránkách (krkal.org), sledujte zda není k dispozici 
   aktualizovaná verze systému Krkal (patch)


HRA KRKAL: OVLÁDÁNÍ
===================

 # pohyb postavièky: kurzorové klávesy 
Postavièka se vdy pohybuje pouze horizontálním nebo vertikálním smìrem
- nikdy ne šikmo. Lze dret i více smìrovıch kláves najednou, poté se 
bude postavièka pohybovat jedním ze smìrù, kam mùe jít. To je 
neocenitelná vlastnost v rùznıch bludištích. 
 #  pøepínání mezi postavièkami: TAB 
v nìkterıch levelech je více postavièek nìkdy je moné mezi postavièkami 
pøepínat, jindy se všechny ovládají najednou (kdy pak napø. stiskne 
šipku vpravo - všechny postavièky pùjdou doprava) 
 # pouití bomby: Z 
 # pouití miny: X 
 # sebrání znaèky: A 
 # poloení (otoèení) smìrové znaèky: 1+klávesa smìru 
 # poloení zákazu vjezdu: 2 
 # poloení šedesátky: 3 
 # poloení konce šedesátky: 4 
 # zpìt do úvodního menu: Esc

   Hráè mùe sbírat nìkteré pøedmìty. Od kadého pøedmìtu mùe mít 
maximálnì 12 kusù. Vìtšina pøedmìtù se sebere sama, jakmile hráè vstoupí
na políèko s pøedmìtem. U tìchto pøedmìtù není moné rozhodnout se, 
jestli je chci nebo ne. Vıjimkou jsou znaèky, ty se sbírají speciální 
klávesou 'A'. 
   Sebrané pøedmìty nelze jen tak zahodit. Nìkteré pøedmìty je moné 
pouít speciální klávesou (viz vıše). Jiné pøedmìty se pouijí samy 
- napø. klíè se pouije vdy, kdy hráè chce vstoupit na políèko,
kde je zámek. Tady bych chtìl upozornit na jednu záludnost - pokud hráè
má klíè a nìjaká síla ho pøinutí vjet (napø. kloue se po ledu) 
na políèko se zámkem - tak se zámek vdy odemkne, a hráè chce nebo ne. 


TECHNICKÁ PODPORA, INFORMACE, NÁPOVÌDA
======================================

Velké mnoství informací najdete pøímo v dokumentaci, která je 
souèástí Krkala. V dokumentaci najdete popis objektù ve høe Krkal, jak
ovládat editor, jak psát skripty... 
Další informace naleznete na našich webovıch stránkách:

	www.krkal.org
	
Námìty, ádosti o rady, pøipomínky, hlášení chyb, ... posílejte do 
našeho internetového fóra: forum.krkal.org
A nakonec mùete kontaktovat nìkterého z autorù pøímo e-mailem.

