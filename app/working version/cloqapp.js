document.addEventListener("DOMContentLoaded", function () {
    const form = document.getElementById("myForm");
    const submitBtn = document.getElementById("submitBtn");
    const myinputs = form.querySelectorAll("input");
    const sliders = document.querySelectorAll(".slider_wrap div");

    var brightness = document.getElementById("brightness"),
        sensor_timer = document.getElementById("sensor_timer"),
        summer_offset = document.getElementById("summer_offset"),
        winter_offset = document.getElementById("winter_offset"),
        cool = document.getElementById("cool"),
        warm = document.getElementById("warm"),
        hot = document.getElementById("hot"),
        inputs = [cool, warm, hot],
        led = [brightness],
        timer = [sensor_timer],
        summOff = [summer_offset],
        wintOff = [winter_offset],
        br = brightness.value,
        st = sensor_timer.value,
        so = summer_offset.value,
        wo = winter_offset.value,
        c = cool.value,
        w = warm.value,
        h = hot.value;

    let originalValues = new Map();

    // Slider configurations
    const sliderConfigs = [
        { id: "slider1", start: [br], range: { min: 0, max: 50 }, inputId: "brightness", padding: [1, 0] },
        { id: "slider2", start: [st], range: { min: 0, max: 15 }, inputId: "sensor_timer", padding: [1, 0] },
        { id: "slider3", start: [c, w, h], range: { min: 0, max: 40 }, inputIds: ["cool", "warm", "hot"], connect: [true, true, true, true], padding: [0, 0] },
        { id: "slider4", start: [so], range: { min: -12, max: 12 }, inputId: "summer_offset", padding: [1, 1] },
        { id: "slider5", start: [wo], range: { min: -12, max: 12 }, inputId: "winter_offset", padding: [1, 1] }
    ];

    // Initialize NoUiSliders
    sliders.forEach((slider, index) => {
        const config = sliderConfigs[index];

        noUiSlider.create(slider, {
            start: config.start,
            range: config.range,
            tooltips: true,
            connect: config.connect || [true, false],
            padding: config.padding || [0, 0], // Disable edges
            format: {
                to: value => Math.round(value),
                from: value => Number(value)
            }
        });

        // Store original values
        originalValues.set(slider, slider.noUiSlider.get());

        // Update corresponding input when slider changes
        if (Array.isArray(config.inputIds)) {
            slider.noUiSlider.on("update", function (values) {
                config.inputIds.forEach((inputId, idx) => {
                    document.getElementById(inputId).value = values[idx];
                });
            });
        } else {
            slider.noUiSlider.on("update", function (values) {
                document.getElementById(config.inputId).value = values;
            });
        }

        // Update slider when input changes
        if (Array.isArray(config.inputIds)) {
            config.inputIds.forEach((inputId, idx) => {
                document.getElementById(inputId).addEventListener("input", function () {
                    let newValues = config.inputIds.map(id => document.getElementById(id).value);
                    slider.noUiSlider.set(newValues);
                    checkChanges();
                });
            });
        } else {
            document.getElementById(config.inputId).addEventListener("input", function () {
                slider.noUiSlider.set(this.value);
                checkChanges();
            });
        }
    });

    // Store initial values for inputs
    myinputs.forEach(input => {
        originalValues.set(input, input.type === "checkbox" || input.type === "radio" ? input.checked : input.value);
    });

    function checkChanges() {
        let changed = false;

        myinputs.forEach(input => {
            if (originalValues.get(input) !== (input.type === "checkbox" || input.type === "radio" ? input.checked : input.value)) {
                changed = true;
            }
        });

        sliders.forEach(slider => {
            if (JSON.stringify(slider.noUiSlider.get()) !== JSON.stringify(originalValues.get(slider))) {
                changed = true;
            }
        });

        if (changed) {
            if (submitBtn.style.display === "none" || submitBtn.style.display === "") {
                submitBtn.style.display = "block"; // Ensure it's visible
                requestAnimationFrame(() => {
                    submitBtn.style.opacity = "1"; // Fade in
                    submitBtn.style.transform = 'scale(1.2)'; // Scale in
                });
            } else {
                // If already visible, just fade in again
                submitBtn.style.opacity = "1";
                submitBtn.style.transform = 'scale(1.2)'; // Scale in
            }
        } else {
            submitBtn.style.opacity = "0"; // Fade out
            submitBtn.style.transform = 'scale(1)'; // Scale out
        }
    }

    myinputs.forEach(input => input.addEventListener("input", checkChanges));
    sliders.forEach(slider => slider.noUiSlider.on("update", checkChanges));
});

// Temperature Toggle Switcher
const celciusRadio = document.getElementById("celcius");
const fahrenheitRadio = document.getElementById("fahrenheit");

const cel = document.getElementById("cel");
const fah = document.getElementById("fah");

function tempToggleBoxes() {
    cel.style.display = celciusRadio.checked ? "block" : "none";
    fah.style.display = fahrenheitRadio.checked ? "block" : "none";
}

document.querySelectorAll('.temp-toggles .radio-input').forEach(radio => {
    radio.addEventListener('change', tempToggleBoxes);
});
// Initialize correct box visibility on page load
tempToggleBoxes();

// DST Toggle Switcher
const summerRadio = document.getElementById("summer");
const winterRadio = document.getElementById("winter");

const sumR = document.getElementById("sumR");
const wintR = document.getElementById("wintR");

function dstToggleBoxes() {
    sumR.style.display = summerRadio.checked ? "block" : "none";
    wintR.style.display = winterRadio.checked ? "block" : "none";
}

document.querySelectorAll('.dst-toggles .radio-input').forEach(radio => {
    radio.addEventListener('change', dstToggleBoxes);
});
// Initialize correct box visibility on page load
dstToggleBoxes();


! function() {
    "use strict";
    document.getElementById("tabs");
    var e = document.querySelectorAll(".tab_panel"),
        t = document.querySelectorAll(".tab_heading"),
        i = [].slice.call(e);
    [].slice.call(t);

    function n(e, i) {
        i.forEach((function(i, n, o) {
            i !== e ? (i.setAttribute("aria-hidden", !0), t[n].setAttribute("aria-selected", !1)) : (i.setAttribute("aria-hidden", !1), t[n].setAttribute("aria-selected", !0))
        }))
    }

    function o(e) {
        var t = e.target.getAttribute("aria-controls");
        n(document.querySelector("#" + t), i), localStorage.setItem("lastTab", t)
    }(t.forEach((function(e, t, i) {
        e.addEventListener("click", o)
    })), localStorage.length > 0) && n(document.getElementById(localStorage.getItem("lastTab")), i)
}();


function removeSpaces(string){return string.split(' ').join('');}
function removeSpaceAfterComma(string){return string.replace(", ", ",");}

Date.prototype.stdTimezoneOffset = function () {
    var jan = new Date(this.getFullYear(), 0, 1);
    var jul = new Date(this.getFullYear(), 6, 1);
    return Math.max(jan.getTimezoneOffset(), jul.getTimezoneOffset());
}  

Date.prototype.isDstObserved = function () {
    return this.getTimezoneOffset() < this.stdTimezoneOffset();
}

var today = new Date();
if (today.isDstObserved()) { 
    dst.classList.add("show");
}

function update(){Math.floor((new Date).getTime()/1e3);document.body.className="darkmode",requestAnimationFrame(update)}requestAnimationFrame(update);

const accordionBtns = document.querySelectorAll(".accordion");
accordionBtns.forEach((accordion) => {
    accordion.onclick = function () {
        this.classList.toggle("is-open");
        let content = this.nextElementSibling;
        console.log(content);
        if (content.style.maxHeight) {
            content.style.maxHeight = null;
        } else {
            content.style.maxHeight = content.scrollHeight + "px";
            console.log(content.style.maxHeight);
        }
    };
});

const fanenContent = document.querySelector('.fanen-content');
const faner = document.querySelectorAll('.fanen');
const tabIndicator = document.querySelector('.fanen-indicator');

faner.forEach((fanen, index) => {
    fanen.addEventListener('click', () => {
        faner.forEach(t => t.classList.remove('active'));
        fanen.classList.add('active');

        if (index === 0) {
            switchToTab(1);
        } else {
            switchToTab(2);
        }
    });
});

// Swipe Support
let startX = 0;
let currentX = 0;
let startY = 0;
let isSwiping = false;
let hasDragged = false;
let dragThreshold = 20; // Minimum movement to confirm dragging

function startSwipe(e) {
    startX = e.touches ? e.touches[0].clientX : e.clientX;
    startY = e.touches ? e.touches[0].clientY : e.clientY;
    isSwiping = true;
    hasDragged = false;
}

function moveSwipe(e) {
    if (!isSwiping) return;

    currentX = e.touches ? e.touches[0].clientX : e.clientX;
    const diffX = currentX - startX;
    const diffY = (e.touches ? e.touches[0].clientY : e.clientY) - startY;

    // Only allow swipe when dragging beyond threshold
    if (Math.abs(diffX) > dragThreshold && Math.abs(diffX) > Math.abs(diffY)) {
        hasDragged = true;
        e.preventDefault(); // Prevent vertical scrolling
        fanenContent.style.transition = 'none';
        fanenContent.style.transform = `translateX(${getCurrentOffset() + diffX}px)`;
    }
}

function endSwipe(e) {
    if (!isSwiping) return;

    const diffX = currentX - startX;

    // If dragging occurred, process swipe logic
    if (hasDragged) {
        if (diffX > 50) {
            switchToTab(1); // Swipe Right → Show Tab 1
        } else if (diffX < -50) {
            switchToTab(2); // Swipe Left → Show Tab 2
        } else {
            resetPosition(); // Reset if swipe is too short
        }
    }

    isSwiping = false;
    hasDragged = false;
}

fanenContent.addEventListener('touchstart', startSwipe, { passive: false });
fanenContent.addEventListener('touchmove', moveSwipe, { passive: false });
fanenContent.addEventListener('touchend', endSwipe);

fanenContent.addEventListener('pointerdown', startSwipe);
fanenContent.addEventListener('pointermove', moveSwipe);
fanenContent.addEventListener('pointerup', endSwipe);

// Utility Functions
function switchToTab(tabNumber) {
    if (tabNumber === 1) {
        fanenContent.classList.remove('tab2-active');
        fanenContent.classList.add('tab1-active');
        tabIndicator.style.transform = 'translateX(0)';
    } else {
        fanenContent.classList.remove('tab1-active');
        fanenContent.classList.add('tab2-active');
        tabIndicator.style.transform = 'translateX(100%)';
    }

    fanenContent.style.transition = 'transform 0.2s ease-in-out';
    fanenContent.style.transform = getCurrentOffset() === 0 ? 'translateX(0)' : 'translateX(-50%)';
}

function getCurrentOffset() {
    return fanenContent.classList.contains('tab1-active') ? 0 : -fanenContent.clientWidth / 2;
}

function resetPosition() {
    fanenContent.style.transition = 'transform 0.2s ease-in-out';
    fanenContent.style.transform = getCurrentOffset() === 0 ? 'translateX(0)' : 'translateX(-50%)';
}

// Function to flip the card
function flipCard() {
    const card = document.getElementById('flipCard');
    card.classList.toggle('flip');
}

// OpenWeatherMap Sunrise/Sunset Widget
document.addEventListener('DOMContentLoaded', function() {
    // Function to initialize the OpenWeatherMap widget
    function initOpenWeatherWidget() {
        // Get form elements
        const locationInput = document.getElementById('location');
        const apiKeyInput = document.getElementById('apikey');
        const submitButton = document.getElementById('submit-location');
        
        // Get display elements for the sunrise/sunset widget
        const sunPath = document.getElementById('curve');
        const titleElement = document.getElementById('title_');
        const sunriseTimeElement = document.getElementById('sunrise-time');
        const sunsetTimeElement = document.getElementById('sunset-time');
        const sunlightHoursElement = document.getElementById('sunlight-hours-time');
        
        // Variables to store data
        let sunriseTime, sunsetTime, timezoneOffset;
        
        // Function to validate location input
        function validateLocationInput(value) {
            // Check if location has a country code (contains a comma)
            return value && value.trim() !== '' && value.includes(',');
        }
        
        // Function to update submit button state
        function updateSubmitButtonState() {
            submitButton.disabled = !validateLocationInput(locationInput.value);
        }
        
        // Format time to display
        function formatTime(date) {
            const hours = String(date.getHours()).padStart(2, '0');
            const minutes = String(date.getMinutes()).padStart(2, '0');
            return `${hours}:${minutes}`;
        }
        
        // Function to position a marker at a specific point along the path
        function positionMarkerAlongPath(markerElement, progress) {
            const path = document.getElementById('curve');
            if (!path || !markerElement) return;
            
            const pathLength = path.getTotalLength();
            const point = path.getPointAtLength(progress * pathLength);
            
            markerElement.setAttribute('cx', point.x);
            markerElement.setAttribute('cy', point.y);
        }
        
        // Set up sun animation with SVG animateMotion, updating every 30 seconds
        function setupSunAnimation() {
            if (!sunriseTime || !sunsetTime) return;
            
            const sunElement = document.getElementById('sunSVG');
            const sunMotion = document.getElementById('sunMotion');
            const sunriseMark = document.getElementById('sunriseMarker');
            const sunsetMark = document.getElementById('sunsetMarker');
            
            if (!sunElement || !sunriseMark || !sunsetMark) {
                console.warn("Missing SVG elements needed for animation");
                return;
            }
            
            // Function to update sun position and animation
            function updateSunPosition() {
                // Get current time relative to day
                const now = new Date();
                const localNow = new Date(now.getTime() + (timezoneOffset * 1000)); // Adjust for timezone
                
                // Calculate seconds since midnight
                const secondsSinceMidnight = localNow.getHours() * 3600 + localNow.getMinutes() * 60 + localNow.getSeconds();
                
                // Calculate the fraction of the day (0 to 1)
                const dayProgress = secondsSinceMidnight / 86400;
                
                // Find existing animateMotion
                let existingMotion = sunElement.querySelector('animateMotion');
                
                // Create a new animateMotion element
                const newMotion = document.createElementNS("http://www.w3.org/2000/svg", "animateMotion");
                newMotion.setAttribute("id", "sunMotion");
                newMotion.setAttribute("begin", "0s");
                newMotion.setAttribute("dur", "24h");
                newMotion.setAttribute("repeatCount", "indefinite");
                newMotion.setAttribute("path", "M0 69.8975C56.1994 69.8975 111.319 3 167.541 3C227.168 3 274.438 69.8975 335 69.8975");
                
                // Set the keyPoints to start from the current time
                newMotion.setAttribute("keyPoints", `${dayProgress};1`);
                newMotion.setAttribute("keyTimes", "0;1");
                newMotion.setAttribute("calcMode", "linear");
                
                // Replace the old motion with the new one
                if (existingMotion) {
                    sunElement.removeChild(existingMotion);
                }
                sunElement.appendChild(newMotion);
                
                // Set sun color based on time of day
                if (localNow >= sunriseTime && localNow < sunsetTime) {
                    sunElement.setAttribute('fill', '#FFC300'); // Daytime
                } else {
                    sunElement.setAttribute('fill', '#777777'); // Nighttime
                }
                
                // Update background color based on time relative to sunrise and sunset
                updateBackgroundColor(localNow);
                
                console.log(`Updated sun position to ${dayProgress.toFixed(4)} of day`);
            }
          
            // Function to update background color based on time
function updateBackgroundColor(currentTime) {
    const boxElement = document.querySelector('.box-large');
    if (!boxElement || !sunriseTime || !sunsetTime) {
      console.log("Missing required elements for background update");
      return;
    }
    
    // Calculate 20 minutes in milliseconds
    const twentyMinutes = 20 * 60 * 1000;
    
    // Calculate time ranges
    const beforeSunrise = new Date(sunriseTime.getTime() - twentyMinutes);
    const afterSunset = new Date(sunsetTime.getTime() + twentyMinutes);
    
    console.log("Current time:", currentTime);
    console.log("Before sunrise time:", beforeSunrise);
    console.log("Sunrise time:", sunriseTime);
    console.log("Sunset time:", sunsetTime);
    console.log("After sunset time:", afterSunset);
    
    // Determine which background color to use
    if (currentTime >= beforeSunrise && currentTime < sunriseTime) {
      // 20 minutes before sunrise (dawn/sunrise transition)
      boxElement.style.backgroundColor = '#ff06bb0a'; // Purplish/pink for dawn
      console.log("Setting dawn transition background (before sunrise)");
    } 
    else if (currentTime >= sunriseTime && currentTime < sunsetTime) {
      // Daytime
      boxElement.style.backgroundColor = '#ffffff0a'; // Light background
      console.log("Setting daytime background");
    } 
    else if (currentTime >= sunsetTime && currentTime < afterSunset) {
      // 20 minutes after sunset (dusk/sunset transition)
      boxElement.style.backgroundColor = '#ff06bb0a'; // Purplish/pink for dusk
      console.log("Setting dusk transition background (after sunset)");
    } 
    else {
      // Nighttime
      boxElement.style.backgroundColor = '#00000026'; // Darker background
      console.log("Setting nighttime background");
    }
  }
            
            // Position the sunrise marker
            const sunriseSeconds = sunriseTime.getHours() * 3600 + sunriseTime.getMinutes() * 60;
            const sunriseProgress = sunriseSeconds / 86400;
            positionMarkerAlongPath(sunriseMark, sunriseProgress);
            
            // Position the sunset marker
            const sunsetSeconds = sunsetTime.getHours() * 3600 + sunsetTime.getMinutes() * 60;
            const sunsetProgress = sunsetSeconds / 86400;
            positionMarkerAlongPath(sunsetMark, sunsetProgress);
            
            // Initial sun position update
            updateSunPosition();
            
            // Clear any existing interval before setting a new one
            if (sunElement.dataset.updateIntervalId) {
                console.log("Clearing existing interval within setupSunAnimation:", sunElement.dataset.updateIntervalId);
                clearInterval(parseInt(sunElement.dataset.updateIntervalId));
            }
            
            // Set up interval to update the sun position every 30 seconds
            const updateIntervalId = setInterval(updateSunPosition, 30000);
            console.log("Created new interval:", updateIntervalId);
            
            // Store the interval ID so it can be cleared if needed
            sunElement.dataset.updateIntervalId = updateIntervalId;
        }
        
        // Function to update temperature displays
function updateTemperatureDisplays(data) {
    if (!data || !data.main || !data.main.temp) {
      console.log("Temperature data not available");
      return;
    }
    
    // Get temperature in Kelvin from API
    const tempKelvin = data.main.temp;
    
    // Convert to Celsius and Fahrenheit
    const tempCelsius = Math.round(tempKelvin - 273.15);
    const tempFahrenheit = Math.round((tempKelvin - 273.15) * 9/5 + 32);
    
    // Update the display elements
    const celsiusDisplay = document.querySelector('#cel h4');
    const fahrenheitDisplay = document.querySelector('#fah h4');
    
    if (celsiusDisplay) {
      celsiusDisplay.innerHTML = `${tempCelsius} <span>C</span>`;
    }
    
    if (fahrenheitDisplay) {
      fahrenheitDisplay.innerHTML = `${tempFahrenheit} <span>F</span>`;
    }
    
    // Update weather icon
    const conditionsElement = document.getElementById('conditions');
    if (conditionsElement && data.weather && data.weather[0]) {
      const weatherIcon = data.weather[0].icon;
      const weatherDescription = data.weather[0].description;
      
      // Get weather icon from OpenWeatherMap
      conditionsElement.innerHTML = `
        <img src="https://openweathermap.org/img/wn/${weatherIcon}@2x.png" 
             alt="${weatherDescription}" 
             title="${weatherDescription}"
             style="width: 50px; height: 50px;">
      `;
      
      console.log(`Weather condition: ${weatherDescription}, icon: ${weatherIcon}`);
    }
    
    console.log(`Updated temperatures: ${tempCelsius}°C / ${tempFahrenheit}°F`);
  }
        
        // Fetch weather data function
        async function fetchWeatherData() {
            try {
                // Clear any existing sun update interval
                const sunElement = document.getElementById('sunSVG');
                if (sunElement && sunElement.dataset.updateIntervalId) {
                    console.log("Clearing existing interval:", sunElement.dataset.updateIntervalId);
                    clearInterval(parseInt(sunElement.dataset.updateIntervalId));
                    sunElement.dataset.updateIntervalId = ''; // Reset the data attribute
                }
                
                // Set loading state
                if (sunriseTimeElement) sunriseTimeElement.textContent = '';
                if (sunsetTimeElement) sunsetTimeElement.textContent = '';
                if (sunlightHoursElement) sunlightHoursElement.textContent = '';
                
                // Disable submit button during fetch
                if (submitButton) submitButton.disabled = true;
                
                // Get input values
                const location = locationInput.value;
                const apiKey = apiKeyInput.value;
                
                console.log("Fetching data for:", location);
                
                // Parse location (city, country)
                const [city, countryCode] = location.split(',').map(part => part.trim());
                
                // Create API URL
                const apiUrl = `https://api.openweathermap.org/data/2.5/weather?q=${city},${countryCode}&appid=${apiKey}`;
                
                // Fetch data
                console.log("Fetching from:", apiUrl);
                const response = await fetch(apiUrl);
                
                if (!response.ok) {
                    throw new Error(`Weather API returned ${response.status}: ${response.statusText}`);
                }
                
                const data = await response.json();
                console.log("Data received:", data);
                
                // Extract sunrise/sunset data
                const sunriseUnix = data.sys.sunrise;
                const sunsetUnix = data.sys.sunset;
                timezoneOffset = data.timezone;
                
                // Convert times to local timezone
                sunriseTime = new Date((sunriseUnix + timezoneOffset) * 1000);
                sunsetTime = new Date((sunsetUnix + timezoneOffset) * 1000);
                
                // Update the UI
                if (titleElement) titleElement.textContent = `${data.name}, ${data.sys.country}`;
                updateSunTimes();
                updateSunlightHours();
                
                // Add this line to update temperature displays
                updateTemperatureDisplays(data);
                
                // Setup the SVG animation
                setupSunAnimation();
                
                // Re-enable submit button and update its state
                updateSubmitButtonState();
                
                return true;
            } catch (error) {
                console.error('Error fetching weather data:', error);
                alert('Failed to fetch weather data. Please check your location input and try again.');
                
                // Re-enable submit button and update its state
                updateSubmitButtonState();
                
                return false;
            }
        }
        
        // Update sunrise and sunset times in the UI
        function updateSunTimes() {
            if (!sunriseTime || !sunsetTime || !sunriseTimeElement || !sunsetTimeElement) return;
            
            const now = new Date();
            const localTimeNow = new Date(now.getTime() + timezoneOffset * 1000);
            const sunriseFormatted = formatTime(sunriseTime);
            const sunsetFormatted = formatTime(sunsetTime);
            
            if (localTimeNow < sunriseTime) {
                if (titleElement) titleElement.innerHTML = `<span id="sunup"></span> Sunrise`;
                sunsetTimeElement.textContent = `${sunriseFormatted}`;
                sunriseTimeElement.innerHTML = `<span id="sundown"></span> ${sunsetFormatted}`;
            } else if (localTimeNow < sunsetTime) {
                const tomorrowSunrise = new Date(sunriseTime);
                tomorrowSunrise.setDate(tomorrowSunrise.getDate() + 1);
                
                if (titleElement) titleElement.innerHTML = `<span id="sundown"></span> Sunset`;
                sunsetTimeElement.textContent = `${sunsetFormatted}`;
                sunriseTimeElement.innerHTML = `<span id="sunup"></span> ${formatTime(tomorrowSunrise)}`;
            } else {
                const tomorrowSunrise = new Date(sunriseTime);
                tomorrowSunrise.setDate(tomorrowSunrise.getDate() + 1);
                
                if (titleElement) titleElement.innerHTML = `<span id="sunup"></span> Sunrise`;
                sunsetTimeElement.textContent = `${formatTime(tomorrowSunrise)}`;
                sunriseTimeElement.innerHTML = `<span id="sundown"></span> ${sunsetFormatted}`;
            }
        }
        
        // Calculate and display total sunlight hours
        function updateSunlightHours() {
            if (!sunriseTime || !sunsetTime || !sunlightHoursElement) return;
            
            const sunlightDuration = (sunsetTime - sunriseTime) / 1000; // Convert to seconds
            const hours = Math.floor(sunlightDuration / 3600);
            const minutes = Math.floor((sunlightDuration % 3600) / 60);
            
            sunlightHoursElement.textContent = `${hours}h ${minutes}m`;
        }
        
        // Add event listeners for input changes and validation
        locationInput.addEventListener('input', updateSubmitButtonState);
        
        // Add event listener for submit button
        submitButton.addEventListener('click', function() {
            if (validateLocationInput(locationInput.value)) {
                fetchWeatherData();
            }
        });
        
        // Add keyup event listener for Enter key
        locationInput.addEventListener('keyup', function(event) {
            if (event.key === 'Enter' && validateLocationInput(this.value)) {
                console.log("Enter pressed, fetching data");
                fetchWeatherData();
            }
        });
        
        // Initialize the submit button state
        updateSubmitButtonState();
        
        // Try to fetch data with initial location value if it's valid
        if (validateLocationInput(locationInput.value)) {
            console.log("Initial location is valid, fetching data");
            fetchWeatherData();
        }
    }
    
    // Initialize the widget when the DOM is loaded
    initOpenWeatherWidget();
});