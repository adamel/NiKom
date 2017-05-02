
static const UBYTE IbmToAmiga[256] = {
    0,  1,  0,'?','?','?','?',  0,  0,  0, 10,  0,  0, 13,  0,  0,
    0,  0,  0,  0,'�','�',  0,  0,  0,  0,  0, 27,  0,  0,  0,  0,
  ' ','!','"','#','$','%','&', 39,'(',')','*','+', 44,'-','.','/',
  '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
  '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
  'P','Q','R','S','T','U','V','W','X','Y','Z','[', 92,']','^','_',
  '`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
  'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~',' ',
  '�','�','�','�','�','�','�','�','�','�','�','�','�','�','�','�',
  '�','�','�','�','�','�','�','�','�','�','�','�','�','�','P','f',
  '�','�','�','�','�','�','�','�','�','?','�','�','�','�','�','�',
  '#','#','#','|','+','+','+','+','+','+','|','+','+','+','+','+',
  '+','+','+','+','-','+','+','+','+','+','+','+','+','-','+','+',
  '+','+','+','+','+','+','+','+','+','+','+','#','#','#','#','#',
  '?','�','?','?','?','?','�','?','?','?','?','?','?','�','?','?',
  '=','�','?','?','?','?','�','~','�','�','�','?','?','�','�',' ',
};

static const UBYTE AmigaToIbm[256] = {
    0,  1,  0,  0,  0,  0,  0,  0,  0,  0, 10,  0,  0, 13,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 27,  0,  0,  0,  0,
  ' ','!','"','#','$','%','&', 39,'(',')','*','+', 44,'-','.','/',
  '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
  '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
  'P','Q','R','S','T','U','V','W','X','Y','Z','[', 92,']','^','_',
  '`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
  'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~',' ',
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  ' ',173,155,156,'?',157,'|', 21,'"','c',166,174,170,'-','r','-',
  248,241,253,'3', 39,230, 20,249, 44,'1',167,175,172,171,'?',168,
  'A','A','A','A',142,143,146,128,'E',144,'E','E','I','I','I','I',
  'D',165,'O','O','O','O',153,'x','?','U','U','U',154,'Y','P',225,
  133,160,131,'a',132,134,145,135,138,130,136,137,141,161,140,139,
  '?',164,149,162,147,'o',148,246,237,151,163,150,129,'y','p',152,
};

static const UBYTE SF7ToAmiga[256] = {
    0,  1,  0,  0,  0,  0,  0,  0,  0,  0, 10,  0,  0, 13,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 27,  0,  0,  0,  0,
  ' ','!','"','#','$','%','&', 39,'(',')','*','+', 44,'-','.','/',
  '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
  '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
  'P','Q','R','S','T','U','V','W','X','Y','Z','�','�','�','^','_',
  '�','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
  'p','q','r','s','t','u','v','w','x','y','z','�','�','�','~',' ',
  '�','�','�','�','�','�','�','�','�','�','�','�','�','�','�','�',
  '�','�','�','�','�','�','�','�','�','�','�','�','�','�','P','f',
  ' ','�','�','�','�','�','�','�','�','�','�','�','�','-','�','�',
  '�','�','�','�','�','�','�','�','�','�','�','�','�','�','�','�',
  '�','�','�','�','�','�','�','�','�','�','�','�','�','�','�','�',
  '�','�','�','�','�','�','�','�','�','�','�','�','�','�','�','�',
  '�','�','�','�','�','�','�','�','�','�','�','�','�','�','�','�',
  '�','�','�','�','�','�','�','�','�','�','�','�','�','�','�','�',
};

static const UBYTE AmigaToSF7[256] = {
    0,  1,  0,  0,  0,  0,  0,  0,  0,  0, 10,  0,  0, 13,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 27,  0,  0,  0,  0,
  ' ','!','"','#','$','%','&', 39,'(',')','*','+', 44,'-','.','/',
  '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
  '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
  'P','Q','R','S','T','U','V','W','X','Y','Z','[', 92,']','^','_',
  '`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
  'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~',  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  ' ','!','c','L','$','Y','I','S','"','c','a','<','?','-','r','-',
  'o','+','2','3', 39,'u','P','.', 44,'1','o','>','?','?','?','�',
  'A','A','A','A','[',']','A','C','E','@','E','E','I','I','I','I',
  'D','N','O','O','O','O', 92,'x','O','U','U','U','?','Y','p','s',
  'a','a','a','a','{','}','a','c','e','e','e','e','i','i','i','i',
  'o','n','o','o','o','o','|','/','o','u','u','u','-','y','P','y',
};

static const UBYTE MacToAmiga[256] = {
    0,  1,  0,  0,  0,  0,  0,  0,  0,  0, 10,  0,  0, 13,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 27,  0,  0,  0,  0,
  ' ','!','"','#','$','%','&', 39,'(',')','*','+', 44,'-','.','/',
  '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
  '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
  'P','Q','R','S','T','U','V','W','X','Y','Z','[', 92,']','^','_',
  '`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
  'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~',' ',
  '�','�','�','�','�','�','�','�','�','�','�','�','�','�','�','�',
  '�','�','�','�','�','�','�','�','�','�','�','�','�','�','�','�',
  '?','�','�','�','�','?','�','�','�','�','?','�','�','?','�','�',
  '?','�','?','?','�','�','?','?','?','?','?','�','�','?','�','�',
  '�','�','�','?','f','~','?','�','�','?',' ','�','�','�','?','?',
  '-','-','"','"','`', 39,'�','?','�',  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

static const UBYTE AmigaToMac[256] = {
    0,  1,  0,  0,  0,  0,  0,  0,  0,  0, 10,  0,  0, 13,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 27,  0,  0,  0,  0,
  ' ','!','"','#','$','%','&', 39,'(',')','*','+', 44,'-','.','/',
  '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
  '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
  'P','Q','R','S','T','U','V','W','X','Y','Z','[', 92,']','^','_',
  '`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
  'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~',' ',
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  ' ',191,162,163,'?',180,'|',164,172,169,187,199,194,'-',168,'-',
  161,177,'2','3',171,181,166,'.', 44,'1',188,200,'?','?','?',192,
  203,'A','A',204,128,129,174,130,'E',131,'E','E','I','I','I','I',
  'D',132,'O','O','O',221,133,'x',175,'U','U','U',134,'Y','?',167,
  136,135,137,139,138,140,190,141,143,142,144,145,147,146,148,149,
  '?',150,152,151,153,155,154,'/',191,157,156,158,159,216,'?','?',
};

static const UBYTE CP850ToAmiga[256] = {
    0,  1,  0,'?','?','?','?',  0,  0,  0, 10,  0,  0, 13,  0,  0,
    0,  0,  0,  0,'�','�',  0,  0,  0,  0,  0, 27,  0,  0,  0,  0,
  ' ','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',
  '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
  '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
  'P','Q','R','S','T','U','V','W','X','Y','Z','[','\\',']','^','_',
  '`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
  'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~',' ',
  '�','�','�','�','�','�','�','�','�','�','�','�','�','�','�','�',
  '�','�','�','�','�','�','�','�','�','�','�','�','�','�','�','f',
  '�','�','�','�','�','�','�','�','�','�','�','�','�','�','�','�',
  '#','#','#','|','+','�','�','�','�','+','|','+','+','�','�','+',
  '+','+','+','+','-','+','�','�','+','+','+','+','+','-','+','�',
  '�','�','�','�','�','i','�','�','�','+','+','#','#','�','�','#',
  '�','�','�','�','�','�','�','�','�','�','�','�','�','�','�','�',
  '�','�','_','�','�','�','�','�','�','�','�','�','�','�','#','�',
};

static const UBYTE AmigaToCP850[256] = {
    0,  1,  0,  0,  0,  0,  0,  0,  0,  0, 10,  0,  0, 13,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 27,  0,  0,  0,  0,
  ' ','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',
  '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
  '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
  'P','Q','R','S','T','U','V','W','X','Y','Z','[','\\',']','^','_',
  '`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
  'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~',127,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  '�','�','�',156,'�','�','�','�','�','�','�','�','�','�','�','�',
  '�','�','�','�','�','�','�','�','�','�','�','�','�','�','�','�',
  '�','�','�','�',142,143,146,128,'�',144,'�','�','�','�','�','�',
  '�','�','�','�','�','�',153,158,157,'�','�','�',154,'�','�','�',
  133,'�',131,'�',132,134,145,135,138,130,136,137,141,'�',140,139,
  '�','�',149,'�',147,'�',148,'�',155,151,'�',150,129,'�','�',152,
};
