// Set the grid for the letters (16x16)
const grid = [
  'T','H','E','J','T','I','M','E','A','I','S','S','H','A','L','F',
	'Q','U','A','R','T','E','R','T','W','E','N','T','Y','O','N','E',
	'T','E','N','S','I','X','T','E','E','N','T','W','O','N','E','I',
	'E','I','G','H','T','E','E','N','F','I','V','E','Z','E','R','O',
	'S','E','V','E','N','T','E','E','N','I','N','E','T','E','E','N',
	'F','O','U','R','T','E','E','N','T','H','I','R','T','E','E','N',
	'T','W','E','L','V','E','L','E','V','E','N','T','H','R','E','E',
	'M','I','N','U','T','E','S','X','P','A','S','T','O','W','O','R',
	'T','W','O','N','E','L','E','V','E','N','I','N','E','T','E','N',
	'S','E','V','E','N','T','H','R','E','E','T','W','E','L','V','E',
	'F','O','U','R','F','I','V','E','I','G','H','T','S','I','X','N',
	'O','C','L','O','C','K','X','M','I','D','N','I','G','H','T','2',
	'N','O','O','N','I','A','N','D','X','O','X','O','X','O','X','O',
	'C','O','L','D','C','O','O','L','W','A','R','M','H','O','T','2',
	'S','U','N','C','L','O','U','D','R','A','I','N','S','N','O','W',
	'0','1','2','3','4','5','0','1','2','3','4','5','6','7','8','9'];

// Set start and end positions for the words
const words = {
  // Hours
  1: [130, 132],
  2: [128, 130],
  3: [149, 153],
  4: [160, 163],
  5: [164, 167],
  6: [172, 174],
  7: [144, 148],
  8: [167, 171],
  9: [137, 140],
  10: [141,143],
  11: [132, 137],
  12: [154, 159],
  // Words
  the: [0, 2],
  time: [4, 7],
  is: [9, 10],
  one: [44, 46],
  two: [42, 44],
  three: [107, 111],
  four: [80, 83],
  five: [56, 59],
  six: [35, 37],
  seven: [64, 68],
  eight: [48, 52],
  nine: [72, 75],
  ten: [32, 34],
  eleven: [101, 106],
  twelve: [96, 101],
  thirteen: [88, 95],
  fourteen: [80, 87],
  quarter: [16, 22],
  sixteen: [35, 41],
  seventeen: [64, 72],
  eighteen: [48, 55],
  nineteen: [72, 79],
  twenty: [23, 28],
  twentyone: [23, 31],
  half: [12, 15],
  past: [120, 123],
  to: [123, 124],
  oclock: [176, 181],
  minute: [112, 117],
  minutes: [112, 118],
  and: [197, 199],
  cool: [212, 215],
  warm: [216, 219],
  noon: [192, 195],
  midnight: [183, 190],
  t_0: [240, 240],
  t_1: [241, 241],
  t_2: [242, 242],
  t_3: [243, 243],
  t_4: [244, 244],
  t_5: [245, 245],
  u_0: [246, 246],
  u_1: [247, 247],
  u_2: [248, 248],
  u_3: [249, 249],
  u_4: [250, 250],
  u_5: [251, 251],
  u_6: [252, 252],
  u_7: [253, 253],
  u_8: [254, 254],
  u_9: [255, 255],
};

const htmlLetters = document.querySelector('.letters');
const letter = htmlLetters.childNodes;

// Add letters into html
grid.map((letter) => {
  htmlLetters.insertAdjacentHTML('beforeend', `<div class='letter'>${letter}</div>`);
});

function setActive(word) {
  for (i = word[0]; i <= word[1]; i++) {
    if (i !== 11) letter[i].classList.add('active');
    if (i >= 212 & i <= 215) letter[i].classList.add('cyan');
  }
}

function setWords() {
  letter.forEach((htmlLetter) => {
    if (htmlLetter.classList.contains('active')) {
      htmlLetter.classList.remove('active');
    }
  });

  //setActive(words.the);
  //setActive(words.time);
  //setActive(words.is);
  setActive(words.and);
  setActive(words.cool);

  const date = new Date();
  let second = date.getSeconds();
  let minute = date.getMinutes();
  let hour = date.getHours();

  minute = minute - (minute % 1);

  hour = hour % 12;
  hour = hour ? hour : 12; // If the hour is 0, set it to 12

  // Set the seconds
  switch (second) {
  case 0:
      setActive(words.t_0);
      setActive(words.u_0);
      break;
	case 1:
      setActive(words.t_0);
      setActive(words.u_1);
      break;
	case 2:
      setActive(words.t_0);
      setActive(words.u_2);
      break;
  case 3:
      setActive(words.t_0);
      setActive(words.u_3);
      break;
	case 4:
      setActive(words.t_0);
      setActive(words.u_4);
      break;
	case 5:
      setActive(words.t_0);
      setActive(words.u_5);
      break;
  case 6:
      setActive(words.t_0);
      setActive(words.u_6);
      break;
	case 7:
      setActive(words.t_0);
      setActive(words.u_7);
      break;
	case 8:
      setActive(words.t_0);
      setActive(words.u_8);
      break;
  case 9:
      setActive(words.t_0);
      setActive(words.u_9);
      break;
	case 10:
      setActive(words.t_1);
      setActive(words.u_0);
      break;
	case 11:
      setActive(words.t_1);
      setActive(words.u_1);
      break;
  case 12:
      setActive(words.t_1);
      setActive(words.u_2);
      break;
	case 13:
      setActive(words.t_1);
      setActive(words.u_3);
      break;
	case 14:
      setActive(words.t_1);
      setActive(words.u_4);
      break;
  case 15:
      setActive(words.t_1);
      setActive(words.u_5);
      break;
	case 16:
      setActive(words.t_1);
      setActive(words.u_6);
      break;
	case 17:
      setActive(words.t_1);
      setActive(words.u_7);
      break;
  case 18:
      setActive(words.t_1);
      setActive(words.u_8);
      break;
	case 19:
      setActive(words.t_1);
      setActive(words.u_9);
      break;
	case 20:
      setActive(words.t_2);
      setActive(words.u_0);
      break;
  case 21:
      setActive(words.t_2);
      setActive(words.u_1);
      break;
	case 22:
      setActive(words.t_2);
      setActive(words.u_2);
      break;
	case 23:
      setActive(words.t_2);
      setActive(words.u_3);
      break;
  case 24:
      setActive(words.t_2);
      setActive(words.u_4);
      break;
	case 25:
      setActive(words.t_2);
      setActive(words.u_5);
      break;
	case 26:
      setActive(words.t_2);
      setActive(words.u_6);
      break;
  case 27:
      setActive(words.t_2);
      setActive(words.u_7);
      break;
	case 28:
      setActive(words.t_2);
      setActive(words.u_8);
      break;
	case 29:
      setActive(words.t_2);
      setActive(words.u_9);
      break;
  case 30:
      setActive(words.t_3);
      setActive(words.u_0);
      break;
	case 31:
      setActive(words.t_3);
      setActive(words.u_1);
      break;
	case 32:
      setActive(words.t_3);
      setActive(words.u_2);
      break;
  case 33:
      setActive(words.t_3);
      setActive(words.u_3);
      break;
	case 34:
      setActive(words.t_3);
      setActive(words.u_4);
      break;
	case 35:
      setActive(words.t_3);
      setActive(words.u_5);
      break;
	case 36:
      setActive(words.t_3);
      setActive(words.u_6);
      break;
  case 37:
      setActive(words.t_3);
      setActive(words.u_7);
      break;
	case 38:
      setActive(words.t_3);
      setActive(words.u_8);
      break;
	case 39:
      setActive(words.t_3);
      setActive(words.u_9);
      break;
  case 40:
      setActive(words.t_4);
      setActive(words.u_0);
      break;
	case 41:
      setActive(words.t_4);
      setActive(words.u_1);
      break;
	case 42:
      setActive(words.t_4);
      setActive(words.u_2);
      break;
  case 43:
      setActive(words.t_4);
      setActive(words.u_3);
      break;
	case 44:
      setActive(words.t_4);
      setActive(words.u_4);
      break;
	case 45:
      setActive(words.t_4);
      setActive(words.u_5);
      break;
	case 46:
      setActive(words.t_4);
      setActive(words.u_6);
      break;
	case 47:
      setActive(words.t_4);
      setActive(words.u_7);
      break;
	case 48:
      setActive(words.t_4);
      setActive(words.u_8);
      break;
  case 49:
      setActive(words.t_4);
      setActive(words.u_9);
      break;
	case 50:
      setActive(words.t_5);
      setActive(words.u_0);
      break;
	case 51:
      setActive(words.t_5);
      setActive(words.u_1);
      break;
  case 52:
      setActive(words.t_5);
      setActive(words.u_2);
      break;
	case 53:
      setActive(words.t_5);
      setActive(words.u_3);
      break;
	case 54:
      setActive(words.t_5);
      setActive(words.u_4);
      break;
	case 55:
      setActive(words.t_5);
      setActive(words.u_5);
      break;
  case 56:
      setActive(words.t_5);
      setActive(words.u_6);
      break;
	case 57:
      setActive(words.t_5);
      setActive(words.u_7);
      break;
	case 58:
      setActive(words.t_5);
      setActive(words.u_8);
      break;
	case 59:
      setActive(words.t_5);
      setActive(words.u_9);
      break;
  }
	
  // Set the word per 1 minute
  switch (minute) {
    case 0:
	  setActive(words.the);
	  setActive(words.time);
	  setActive(words.is);
      setActive(words.oclock);
      break;
    case 1:
    case 59:
      setActive(words.one);
      setActive(words.minute);
      break;
    case 2:
    case 58:
      setActive(words.two);
      setActive(words.minutes);
      break;
    case 3:
    case 57:
      setActive(words.three);
      setActive(words.minutes);
      break;
    case 4:
    case 56:
      setActive(words.four);
      setActive(words.minutes);
      break;
    case 5:
    case 55:
      setActive(words.five);
      break;
    case 6:
    case 54:
      setActive(words.six);
      setActive(words.minutes);
      break;
    case 7:
    case 53:
      setActive(words.seven);
      setActive(words.minutes);
      break;
    case 8:
    case 52:
      setActive(words.eight);
      setActive(words.minutes);
      break;
    case 9:
    case 51:
      setActive(words.nine);
      setActive(words.minutes);
      break;
    case 10:
    case 50:
      setActive(words.ten);
      break;
    case 11:
    case 49:
      setActive(words.eleven);
      setActive(words.minutes);
      break;
    case 12:
    case 48:
      setActive(words.twelve);
      setActive(words.minutes);
      break;
    case 13:
    case 47:
      setActive(words.thirteen);
      setActive(words.minutes);
      break;
    case 14:
    case 46:
      setActive(words.fourteen);
      setActive(words.minutes);
      break;
    case 15:
    case 45:
      setActive(words.quarter);
      break;
    case 16:
    case 44:
      setActive(words.sixteen);
      setActive(words.minutes);
      break;
    case 17:
    case 43:
      setActive(words.seventeen);
      setActive(words.minutes);
      break;
    case 18:
    case 42:
      setActive(words.eighteen);
      setActive(words.minutes);
      break;
    case 19:
    case 41:
      setActive(words.nineteen);
      setActive(words.minutes);
      break;
    case 20:
    case 40:
      setActive(words.twenty);
      break;
    case 21:
    case 39:
      setActive(words.twentyone);
      setActive(words.minutes);
      break;
    case 22:
    case 38:
      setActive(words.twenty);
      setActive(words.two);
      setActive(words.minutes);
      break;
    case 23:
    case 37:
      setActive(words.twenty);
      setActive(words.three);
      setActive(words.minutes);
      break;
    case 24:
    case 36:
      setActive(words.twenty);
      setActive(words.four);
      setActive(words.minutes);
      break;
    case 25:
    case 35:
      setActive(words.twenty);
      setActive(words.five);
      break;
    case 26:
    case 34:
      setActive(words.twenty);
      setActive(words.six);
      setActive(words.minutes);
      break;
    case 27:
    case 33:
      setActive(words.twenty);
      setActive(words.seven);
      setActive(words.minutes);
      break;
    case 28:
    case 32:
      setActive(words.twenty);
      setActive(words.eight);
      setActive(words.minutes);
      break;
    case 29:
    case 31:
      setActive(words.twenty);
      setActive(words.nine);
      setActive(words.minutes);
      break;
    case 30:
      setActive(words.half);
      break;
  }

  if (minute < 31) {
    // Ignore past and to when it is exactly half and full
    if (minute !== 0) setActive(words.past);
  } else {
    setActive(words.to);
    hour = hour === 12 ? 1 : hour + 1; // If it is 11:40 you would see "twenty to twelve"
  }
  if (hour == 0 & minute == 0) {
    setActive(words.midnight);
    }
  
  if (hour == 12 & minute == 0) {
    setActive(words.noon);
    }

  // Set hour word
  setActive(words[hour]);
}

// Display the words
setWords();
setInterval(() => setWords(), 1000);
