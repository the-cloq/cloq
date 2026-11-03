
// Dutch Word Clock - Full JavaScript
const API_KEY = 'cda9f4fd8763d0485f77a3eba158c2bc';
let currentLocation = 'London, GB';
let weatherData = null;
let mode = 1; // 1 = word clock, 2 = digital temp
let tempIsCelsius = true;

// 16x16 matrix from your image
const clockMatrix = [
    ['K','W','A','R','T','X','T','W','A','A','L','F','H','A','L','F'],
    ['V','I','E','R','T','I','E','N','E','G','E','N','T','I','E','N'],
    ['A','C','H','T','T','I','E','N','T','W','E','E','D','R','I','E'],
    ['Z','E','V','E','N','T','I','E','N','Z','E','S','T','I','E','N'],
    ['E','L','F','D','E','R','T','I','E','N','V','I','J','F','X','K'],
    ['X','X','Ë','E','N','E','N','T','W','I','N','T','I','G','X','W'],
    ['X','T','W','E','E','X','Ë','N','T','W','I','N','T','I','G','A'],
    ['M','I','N','U','U','T','X','O','V','E','R','X','V','O','O','R'],
    ['T','W','E','E','D','R','I','E','V','I','E','R','Z','O','N','T'],
    ['Z','E','S','Z','E','V','E','N','E','G','E','N','V','I','J','F'],
    ['E','E','N','T','I','E','N','E','L','F','T','W','A','A','L','F'],
    ['X','A','C','H','T','X','U','U','R','S','X','N','A','C','T','S'],
    ['M','I','D','D','A','G','S','X','H','E','L','D','E','R','E','N'],
    ['K','O','U','D','K','O','E','L','W','A','R','M','H','E','E','T'],
    ['X','R','E','G','E','N','X','S','N','E','E','U','W','O','L','K'],
    ['0','1','2','3','4','5','0','1','2','3','4','5','6','7','8','9']
];

// Word positions in the matrix
const words = {
    // Top: "quarter", "half"
    'KWART': [[0,0],[0,1],[0,2],[0,3],[0,4]],
    'HALF': [[0,12],[0,13],[0,14],[0,15]],

    // Minute numbers (rows 1–8, ONLY for minutes)
    /* 01 */ 'EEN_m': [[5,2],[5,3],[5,4]],
    /* 02 */ 'TWEE_m': [[2,8],[2,9],[2,10],[2,11]],
    /* 03 */ 'DRIE_m': [[2,12],[2,13],[2,14],[2,15]],
    /* 04 */ 'VIER_m': [[1,0],[1,1],[1,2],[1,3]],
    /* 05 */ 'VIJF_m': [[4,10],[4,11],[4,12],[4,13]],
    /* 06 */ 'ZES_m': [[3,9],[3,10],[3,11]],
    /* 07 */ 'ZEVEN_m': [[3,0],[3,1],[3,2],[3,3],[3,4]],
    /* 08 */ 'ACHT_m': [[2,0],[2,1],[2,2],[2,3]],
    /* 09 */ 'NEGEN_m': [[1,7],[1,8],[1,9],[1,10],[1,11]],
    /* 10 */ 'TIEN_m': [[3,5],[3,6],[3,7],[3,8]],
    /* 11 */ 'ELF_m': [[4,0],[4,1],[4,2]],
    /* 12 */ 'TWAALF_m': [[0,6],[0,7],[0,8],[0,9],[0,10],[0,11]],
    /* 13 */ 'DERTIEN_m': [[4,3],[4,4],[4,5],[4,6],[4,7],[4,8],[4,9]],
    /* 14 */ 'VIERTIEN_m': [[1,0],[1,1],[1,2],[1,3],[1,4],[1,5],[1,6],[1,7]],
    /* 16 */ 'ZESTIEN_m': [[3,9],[3,10],[3,11],[3,12],[3,13],[3,14],[3,15]],
    /* 17 */ 'ZEVENTIEN_m': [[3,0],[3,1],[3,2],[3,3],[3,4],[3,5],[3,6],[3,7],[3,8]],
    /* 18 */ 'ACHTTIEN_m': [[2,0],[2,1],[2,2],[2,3],[2,4],[2,5],[2,6],[2,7]],
    /* 19 */ 'NEGENTIEN_m': [[1,7],[1,8],[1,9],[1,10],[1,11],[1,12],[1,13],[1,14],[1,15]],
    /* 20 */ 'TWINTIG_m': [[5,7],[5,8],[5,9],[5,10],[5,11],[5,12],[5,13]],
    /* 21 */ 'EENENTWINTIG_m': [[6,1],[6,2],[6,3],[6,4],[6,5],[6,6],[6,7],[6,8],[6,9],[6,10],[6,11],[6,12],[6,13],[6,14]],
    /* 22 */ 'TWEEENTWINTIG_m': [[6,1],[6,2],[6,2],[6,3],[6,4],[6,5],[6,6],[6,7],[6,8],[6,9],[6,10],[6,11],[6,12],[6,13],[6,14]],
    /* 23 */ 'DRIEENTWINTIG_m': [[2,12],[2,13],[2,14],[2,15],[5,7],[5,8],[5,9],[5,10],[5,11],[5,12],[5,13]],
    /* 24 */ 'VIERENTWINTIG_m': [[1,0],[1,1],[1,2],[1,3],[5,7],[5,8],[5,9],[5,10],[5,11],[5,12],[5,13]],
    /* 25 */ 'VIJFENTWINTIG_m': [[4,10],[4,11],[4,12],[4,13],[5,7],[5,8],[5,9],[5,10],[5,11],[5,12],[5,13]],
    /* 26 */ 'ZESENTWINTIG_m': [[3,9],[3,10],[3,11],[5,7],[5,8],[5,9],[5,10],[5,11],[5,12],[5,13]],
    /* 27 */ 'ZEVENENTWINTIG_m': [[3,0],[3,1],[3,2],[3,3],[3,4],[5,7],[5,8],[5,9],[5,10],[5,11],[5,12],[5,13]],
    /* 28 */ 'ACHTENTWINTIG_m': [[2,0],[2,1],[2,2],[2,3],[5,7],[5,8],[5,9],[5,10],[5,11],[5,12],[5,13]],
    /* 29 */ 'NEGENENTWINTIG_m': [[1,7],[1,8],[1,9],[1,10],[1,11],[5,7],[5,8],[5,9],[5,10],[5,11],[5,12],[5,13]],

    'MINUUT': [[7,0],[7,1],[7,2],[7,3],[7,4],[7,5]],
    'OVER': [[7,7],[7,8],[7,9],[7,10]],
    'VOOR': [[7,12],[7,13],[7,14],[7,15]],

    // Hour words (rows 9–11, ONLY for hours)
    /* 01 */ 'EEN_h': [[10,0],[10,1],[10,2]],
    /* 02 */ 'TWEE_h': [[10,0],[10,1],[10,2]],
    /* 03 */ 'DRIE_h': [[10,0],[10,1],[10,2]],
    /* 04 */ 'VIER_h': [[10,0],[10,1],[10,2]],
    /* 05 */ 'VIJF_h': [[9,12],[9,13],[9,14],[9,15]],
    /* 06 */ 'ZES_h': [[9,0],[9,1],[9,2]],
    /* 07 */ 'ZEVEN_h': [[9,3],[9,4],[9,5],[9,6],[9,7]],
    /* 08 */ 'ACHT_h': [[11,1],[11,2],[11,3],[11,4]],
    /* 09 */ 'NEGEN_h': [[9,7],[9,8],[9,9],[9,10],[9,11]],
    /* 10 */ 'TIEN_h': [[10,3],[10,4],[10,5],[10,6]],
    /* 11 */ 'ELF_h': [[10,7],[10,8],[10,9]],
    /* 12 */ 'TWAALF_h': [[10,10],[10,11],[10,12],[10,13],[10,14],[10,15]],
    /* HR */ 'UUR': [[11,6],[11,7],[11,8]],

    // Special/other
    'ZON': [[8,12],[8,13],[8,14]],
    'MIDDAGS': [[12,0],[12,1],[12,2],[12,3],[12,4],[12,5],[12,6]],
    'HELDEREN': [[12,8],[12,9],[12,10],[12,11],[12,12],[12,13],[12,14],[12,15]],
    'EN': [[12,14],[12,15]],
    'KOUD': [[13,0],[13,1],[13,2],[13,3]],
    'KOEL': [[13,4],[13,5],[13,6],[13,7]],
    'WARM': [[13,8],[13,9],[13,10],[13,11]],
    'HEET': [[13,12],[13,13],[13,14],[13,15]],
    'REGEN': [[14,1],[14,2],[14,3],[14,4],[14,5]],
    'SNEEUW': [[14,7],[14,8],[14,9],[14,10],[14,11]],
    'WOLK': [[14,12],[14,13],[14,14],[14,15]],
};


// 7-segment digit map: which segments are on for each digit
const digitalDigits = {
  0: [
    [0,1],[0,2],[0,3],
    [1,0],[1,4],
    [2,0],[2,4],
    [3,0],[3,4],
    [4,0],[4,4],
    [5,0],[5,4],
    [6,1],[6,2],[6,3]
  ],
  1: [
    [0,3],
    [1,2],[1,3],
    [2,3],
    [3,3],
    [4,3],
    [5,3],
    [6,3]
  ],
  2: [
    [0,1],[0,2],[0,3],
    [1,0],[1,4],
    [2,4],
    [3,2],[3,3],
    [4,1],
    [5,0],
    [6,0],[6,1],[6,2],[6,3],[6,4]
  ],
  3: [
    [0,1],[0,2],[0,3],
    [1,0],[1,4],
    [2,4],
    [3,2],[3,3],
    [4,4],
    [5,0],[5,4],
    [6,1],[6,2],[6,3]
  ],
  4: [
    [0,3],
    [1,2],[1,3],
    [2,1],[2,3],
    [3,0],[3,3],
    [4,0],[4,1],[4,2],[4,3],[4,4],
    [5,3],
    [6,3]
  ],
  5: [
    [0,0],[0,1],[0,2],[0,3],[0,4],
    [1,0],
    [2,0],
    [3,1],[3,2],[3,3],
    [4,4],
    [5,0],[5,4],
    [6,1],[6,2],[6,3]
  ],
  6: [
    [0,2],[0,3],
    [1,1],
    [2,0],
    [3,0],[3,1],[3,2],[3,3],
    [4,0],[4,4],
    [5,0],[5,4],
    [6,1],[6,2],[6,3]
  ],
  7: [
    [0,0],[0,1],[0,2],[0,3],[0,4],
    [1,4],
    [2,3],
    [3,2],
    [4,1],
    [5,0],
    [6,0]
  ],
  8: [
    [0,1],[0,2],[0,3],
    [1,0],[1,4],
    [2,0],[2,4],
    [3,1],[3,2],[3,3],
    [4,0],[4,4],
    [5,0],[5,4],
    [6,1],[6,2],[6,3]
  ],
  9: [
    [0,1],[0,2],[0,3],
    [1,0],[1,4],
    [2,0],[2,4],
    [3,1],[3,2],[3,3],[3,4],
    [4,4],
    [5,4],
    [6,1],[6,2],[6,3]
  ]
};

// Segment offsets relative to digit's top-left (row, col)
const segmentOffsets = [
    [[0,1],[0,2],[0,3],[0,4]],      // 0: top
    [[1,5],[2,5]],                  // 1: top-right
    [[3,5],[4,5]],                  // 2: bottom-right
    [[4,1],[4,2],[4,3],[4,4]],      // 3: bottom
    [[3,0],[4,0]],                  // 4: bottom-left
    [[1,0],[2,0]],                  // 5: top-left
    [[2,1],[2,2],[2,3],[2,4]]       // 6: middle
];

// Degree symbol: 3x3 grid at (row 0–2, col 12–14)
const degreeCoords = [];
for(let r=0; r<3; r++) {
  for(let c=12; c<=14; c++) {
    // Exclude the center cell (1,13)
    if (!(r === 1 && c === 13)) {
      degreeCoords.push([r, c]);
    }
  }
}

function lightDigit(num, topRow, leftCol) {
    const segments = digitSegments[num];
    segments.forEach(seg => {
        segmentOffsets[seg].forEach(([dr,dc]) => {
            const cell = document.getElementById(`cell-${topRow+dr}-${leftCol+dc}`);
            if(cell) cell.classList.add('lit');
        });
    });
}

function lightDigitalDigit(num, topRow, leftCol) {
  const coords = digitalDigits[num];
  coords.forEach(([dr, dc]) => {
    const cell = document.getElementById(`cell-${topRow+dr}-${leftCol+dc}`);
    if (cell) cell.classList.add('lit');
  });
}


function lightDegreeSymbol() {
  degreeCoords.forEach(([r, c]) => {
    const cell = document.getElementById(`cell-${r}-${c}`);
    if (cell) cell.classList.add('lit');
  });
}


function renderDigitalTemp(temp) {
  clearAllLights();
  temp = Math.round(temp);
  if (temp < 0) temp = 0;
  if (temp > 99) temp = 99;
  let tens = Math.floor(temp / 10);
  let units = temp % 10;
  if (temp < 10) tens = 0;
  // Tens at (0,0), units at (0,6)
  lightDigitalDigit(tens, 0, 0);
  lightDigitalDigit(units, 0, 6);
  // Degree symbol at (0,12)
  lightDegreeSymbol();
}





function createClockGrid() {
    const grid = document.getElementById('clockGrid');
    grid.innerHTML = '';
    for (let row = 0; row < 16; row++) {
        for (let col = 0; col < 16; col++) {
            const letter = document.createElement('div');
            letter.className = 'letter';
            letter.textContent = clockMatrix[row][col] || '';
            letter.id = `cell-${row}-${col}`;
            grid.appendChild(letter);
        }
    }
}

function clearAllLights() {
    document.querySelectorAll('.letter').forEach(letter => {
        letter.className = 'letter';
    });
}

function lightWord(wordName, colorClass = 'lit') {
    if (words[wordName]) {
        words[wordName].forEach(([row, col]) => {
            const cell = document.getElementById(`cell-${row}-${col}`);
            if (cell) {
                cell.classList.add(colorClass);
            }
        });
    }
}

// --- Time logic ---

function getCurrentLocationTime() {
    if (!weatherData) return new Date();
    const now = new Date();
    const utc = now.getTime() + (now.getTimezoneOffset() * 60000);
    return new Date(utc + (weatherData.timezone * 1000));
}

// Minute words (rows 1–8) for minutes only, hour words (rows 9–11) for hours only
function getMinuteWord(minute) {
    const minuteWords = {
        1: 'EEN_m',
        2: 'TWEE_m',
        3: 'DRIE_m',
        4: 'VIER_m',
        5: 'VIJF_m',
        6: 'ZES_m',
        7: 'ZEVEN_m',
        8: 'ACHT_m',
        9: 'NEGEN_m',
        10: 'TIEN_m',
        11: 'ELF_m',
        12: 'TWAALF_m',
        13: 'DERTIEN_m',
        14: 'VIERTIEN_m',
        15: 'KWART_m',
        16: 'ZESTIEN_m',
        17: 'ZEVENTIEN_m',
        18: 'ACHTTIEN_m',
        19: 'NEGENTIEN_m',
        20: 'TWINTIG_m',
        21: 'EENENTWINTIG_m',
        22: 'TWEEENTWINTIG_m',
        23: 'DRIEENTWINTIG_m',
        24: 'VIERENTWINTIG_m',
        25: 'VIJFENTWINTIG_m',
        26: 'ZESENTWINTIG_m',
        27: 'ZEVENENTWINTIG_m',
        28: 'ACHTENTWINTIG_m',
        29: 'NEGENENTWINTIG_m'
    };
    return minuteWords[minute];
}

function getHourWord(hour) {
    const hourWords = {
        1: 'EEN_h',
        2: 'TWEE_h',
        3: 'DRIE_h',
        4: 'VIER_h',
        5: 'VIJF_h',
        6: 'ZES_h',
        7: 'ZEVEN_h',
        8: 'ACHT_h',
        9: 'NEGEN_h',
        10: 'TIEN_h',
        11: 'ELF_h',
        12: 'TWAALF_h'
    };
    return hourWords[hour];
}

function getTimeWords(date) {
    let hour = date.getHours() % 12 || 12; // 12-hour format
    let minute = date.getMinutes();
    const wordsToLight = [];

    // Special cases for noon and midnight
    if (hour === 12 && minute === 0 && date.getHours() < 12) {
        wordsToLight.push('TWAALF', 'UUR', 'MIDDAGS');
        return wordsToLight;
    }
    if (hour === 12 && minute === 0 && date.getHours() >= 12) {
        wordsToLight.push('TWAALF', 'UUR', 'MIDNACHT');
        return wordsToLight;
    }

    // On the hour
    if (minute === 0) {
        wordsToLight.push(getHourWord(hour));
        wordsToLight.push('UUR');
        return wordsToLight;
    }

    // 1–29: "[MINUTES] [MINUUT] OVER [HOUR]"
    if (minute >= 1 && minute <= 29) {
        if (minute === 15) {
            wordsToLight.push('KWART', 'OVER', getHourWord(hour));
        } else {
            wordsToLight.push(getMinuteWord(minute));
            if (minute % 5 !== 0) wordsToLight.push('MINUUT');
            wordsToLight.push('OVER', getHourWord(hour));
        }
        return wordsToLight;
    }

    // 30: "HALF" + next hour
    if (minute === 30) {
        wordsToLight.push('HALF', getHourWord((hour % 12) + 1));
        return wordsToLight;
    }

    // 31–59: "[MINUTES TO] [MINUUT] VOOR [NEXT HOUR]"
    let minutesTo = 60 - minute;
    let nextHour = (hour % 12) + 1;
    if (minutesTo === 15) {
        wordsToLight.push('KWART', 'VOOR', getHourWord(nextHour));
    } else {
        wordsToLight.push(getMinuteWord(minutesTo));
        if (minutesTo % 5 !== 0) wordsToLight.push('MINUUT');
        wordsToLight.push('VOOR', getHourWord(nextHour));
    }
    return wordsToLight;
}

// --- Weather logic ---
function getWeatherWords(weatherData) {
    const temp = weatherData.main.temp;
    const weatherCondition = weatherData.weather[0].main.toLowerCase();
    const wordsToLight = [];

    if (mode === 1) {
        wordsToLight.push('EN');
        if (temp < 5) {
            wordsToLight.push({ word: 'KOUD', color: 'cold' });
        } else if (temp < 15) {
            wordsToLight.push({ word: 'KOEL', color: 'cool' });
        } else if (temp < 27) {
            wordsToLight.push({ word: 'WARM', color: 'warm' });
        } else {
            wordsToLight.push({ word: 'HEET', color: 'hot' });
        }
    }
    if (mode === 2) {
        // Get current time in UTC seconds
        const now = Math.floor(Date.now() / 1000);
        // Get sunrise and sunset times from weatherData (also in UTC seconds)
        const sunrise = weatherData.sys.sunrise;
        const sunset = weatherData.sys.sunset;

        // If it's night (after sunset or before sunrise), always show ZON
        if (now < sunrise || now > sunset) {
            wordsToLight.push('HELDEREN');
        } else {
            // Otherwise, show weather condition as usual
            if (weatherCondition.includes('clear')) {
                wordsToLight.push('ZON');
            } else if (weatherCondition.includes('rain') || weatherCondition.includes('drizzle')) {
                wordsToLight.push('REGEN');
            } else if (weatherCondition.includes('snow')) {
                wordsToLight.push('SNEEUW');
            } else if (weatherCondition.includes('cloud') || weatherCondition.includes('mist')) {
                wordsToLight.push('WOLK');
            }
        }
    }

    return wordsToLight;
}




// --- API and update logic ---

async function getWeatherData(location) {
    try {
        const response = await fetch(
            `https://api.openweathermap.org/data/2.5/weather?q=${location}&appid=${API_KEY}&units=metric`
        );
        if (!response.ok) throw new Error('Locatie niet gevonden');
        return await response.json();
    } catch (error) {
        document.getElementById('status').textContent = `Error: ${error.message}`;
        return null;
    }
}

function setGridCursor() {
  const grid = document.getElementById('clockGrid');
  if (mode === 2) {
    grid.classList.add('digital-cursor');
  } else {
    grid.classList.remove('digital-cursor');
  }
}

async function updateClock() {
    try {
        if (!weatherData) {
            weatherData = await getWeatherData(currentLocation);
            if (!weatherData) return;
        }
        const locationTime = getCurrentLocationTime();

        clearAllLights();

        if (mode === 1) {
            // --- Mode 1: Word clock + temperature words ---
            // Light up time words
            const timeWords = getTimeWords(locationTime);
            timeWords.forEach(word => lightWord(word));
            // Light up temperature words
            const weatherWords = getWeatherWords(weatherData);
            weatherWords.forEach(item => {
                if (typeof item === 'string') {
                    lightWord(item);
                } else {
                    lightWord(item.word, item.color);
                }
            });
            // Highlight seconds
            highlightSeconds(locationTime);
            // Status bar
            const timeStr = locationTime.toLocaleTimeString('nl-NL', { hour: '2-digit', minute: '2-digit', second: '2-digit' });
            document.getElementById('status').textContent =
                `${weatherData.name}, ${weatherData.sys.country} - ${Math.round(weatherData.main.temp)}°C - ${timeStr}`;
        } else if (mode === 2) {
            // --- Mode 2: Digital temperature + weather condition words ---
            let temp = weatherData.main.temp;
            if (!tempIsCelsius) temp = temp * 9/5 + 32;
            renderDigitalTemp(temp); // <-- You must have this function for digital display!
            // Light up weather condition words only
            const weatherWords = getWeatherWords(weatherData);
            weatherWords.forEach(item => {
                if (typeof item === 'string') {
                    lightWord(item);
                } else {
                    lightWord(item.word, item.color);
                }
            });
            // Highlight seconds (optional in digital mode)
            //highlightSeconds(locationTime);
            // Status bar
            document.getElementById('status').textContent =
                `Temperature: ${Math.round(temp)}°${tempIsCelsius ? 'C' : 'F'} (tap display to toggle)`;
        }
    } catch (error) {
        document.getElementById('status').textContent = `Error: ${error.message}`;
    }
    setGridCursor();
}



function highlightSeconds(date) {
    // Clear previous second highlights
    for (let col = 0; col < 16; col++) {
        const cell = document.getElementById(`cell-15-${col}`);
        if (cell) cell.classList.remove('sec-lit');
    }

    const seconds = date.getSeconds();
    const tens = Math.floor(seconds / 10);
    const units = seconds % 10;

    // Highlight tens (columns 0-5)
    const tensCell = document.getElementById(`cell-15-${tens}`);
    if (tensCell) tensCell.classList.add('sec-lit');

    // Highlight units (columns 6-15)
    const unitsCell = document.getElementById(`cell-15-${6 + units}`);
    if (unitsCell) unitsCell.classList.add('sec-lit');
}

async function updateLocation() {
    const input = document.getElementById('locationInput');
    currentLocation = input.value.trim() || 'London, GB';
    weatherData = null;
    await updateClock();
}

// --- Initialization ---

createClockGrid();
updateClock();
setInterval(updateClock, 1000);
setInterval(() => { weatherData = null; }, 600000);

document.getElementById('locationInput').addEventListener('keypress', function(e) {
    if (e.key === 'Enter') updateLocation();
});

document.getElementById('submitBtn').addEventListener('click', updateLocation);

document.getElementById('modeToggle').onclick = function() {
    mode = mode === 1 ? 2 : 1;
    updateClock();
};

document.getElementById('clockGrid').addEventListener('click', function(){
    if(mode === 2) {
        tempIsCelsius = !tempIsCelsius;
        updateClock();
    }
});
