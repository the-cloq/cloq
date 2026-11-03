/**
 * Light Emission Tracker - Client-side JavaScript
 * 
 * This file contains the JavaScript code to integrate with the ESP8266 device
 * and display the light emission data on the web interface.
 * 
 * Add this code to your main JavaScript file.
 */

// Configuration
const ESP_HOST = window.location.hostname === 'localhost' ? 'ESP_IP_ADDRESS' : window.location.hostname;
const API_BASE_URL = `http://${ESP_HOST}`;
const REFRESH_INTERVAL_TODAY = 60000; // 1 minute
const REFRESH_INTERVAL_HISTORY = 300000; // 5 minutes

// DOM elements
let chartContainer = null;
let lightEmissionChart = null;

// Data state
let todayUptime = 0;
let uptimeHistory = [0, 0, 0, 0, 0, 0, 0]; // Last 6 days + today
let currentDay = new Date().getDay(); // 0 = Sunday, 1 = Monday, ...

// Initialize the light emission tracker
function initLightEmissionTracker() {
  console.log('Initializing Light Emission Tracker');
  
  // Create the chart container if it doesn't exist
  if (!document.getElementById('light-emission-chart')) {
    chartContainer = document.createElement('div');
    chartContainer.id = 'light-emission-chart';
    chartContainer.className = 'light-emission-chart';
    document.querySelector('main') || document.body.appendChild(chartContainer);
  } else {
    chartContainer = document.getElementById('light-emission-chart');
  }
  
  // Fetch initial data
  fetchTodayUptime();
  fetchUptimeHistory();
  
  // Set up refresh intervals
  setInterval(fetchTodayUptime, REFRESH_INTERVAL_TODAY);
  setInterval(fetchUptimeHistory, REFRESH_INTERVAL_HISTORY);
}

// Fetch today's uptime from the ESP8266
async function fetchTodayUptime() {
  try {
    const response = await fetch(`${API_BASE_URL}/api/todayUptime`);
    if (!response.ok) {
      throw new Error(`HTTP error ${response.status}`);
    }
    
    const data = await response.json();
    todayUptime = data.minutes;
    
    // Update today's value in the history array (last position)
    uptimeHistory[6] = todayUptime;
    
    // Update the chart if it exists
    updateChart();
    
    console.log('Updated today uptime:', todayUptime);
  } catch (error) {
    console.error('Error fetching today uptime:', error);
  }
}

// Fetch the 7-day uptime history from the ESP8266
async function fetchUptimeHistory() {
  try {
    const response = await fetch(`${API_BASE_URL}/api/uptimeHistory`);
    if (!response.ok) {
      throw new Error(`HTTP error ${response.status}`);
    }
    
    const data = await response.json();
    
    // Update the history with the last 6 days
    for (let i = 0; i < 6; i++) {
      uptimeHistory[i] = data.days[i];
    }
    
    // Today's value should be from the more frequently updated todayUptime
    uptimeHistory[6] = todayUptime;
    
    // Get current day of week for labels
    currentDay = new Date().getDay();
    
    // Update the chart if it exists
    updateChart();
    
    console.log('Updated uptime history:', uptimeHistory);
  } catch (error) {
    console.error('Error fetching uptime history:', error);
  }
}

// Update or create the chart with current data
function updateChart() {
  // If LightEmissionChart component is already imported and used
  // in your React app, this function might not be needed as the
  // component will update itself with the new data.
  
  // For non-React implementations, you would update the chart here
  // by manipulating the DOM or using a charting library
  if (typeof renderLightEmissionChart === 'function') {
    // If you have a custom chart render function
    renderLightEmissionChart(uptimeHistory, currentDay);
  } else {
    // Basic DOM-based rendering example
    renderBasicChart();
  }
}

// Basic DOM-based chart rendering (for non-React implementations)
function renderBasicChart() {
  // Clear existing content
  chartContainer.innerHTML = '';
  
  // Create chart title
  const title = document.createElement('h3');
  title.className = 'chart-title';
  title.textContent = '7 Day Light Emission';
  chartContainer.appendChild(title);
  
  // Create bars container
  const barsContainer = document.createElement('div');
  barsContainer.className = 'bars-container';
  chartContainer.appendChild(barsContainer);
  
  // Calculate max value for scaling
  const maxValue = Math.max(...uptimeHistory, 1);
  
  // Day names
  const shortDays = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
  
  // Create bars
  uptimeHistory.forEach((minutes, index) => {
    const barWrapper = document.createElement('div');
    barWrapper.className = 'bar-wrapper';
    
    const bar = document.createElement('div');
    bar.className = 'bar';
    if (index === 6) bar.classList.add('today');
    
    // Calculate height percentage
    const heightPercent = Math.max((minutes / maxValue) * 100, 1);
    bar.style.height = `${heightPercent}%`;
    
    // Calculate which day it is
    const dayIndex = (currentDay - 6 + index + 7) % 7;
    const dayLabel = document.createElement('div');
    dayLabel.className = 'day-label';
    dayLabel.textContent = index === 6 ? 'Today' : shortDays[dayIndex];
    
    barWrapper.appendChild(bar);
    barWrapper.appendChild(dayLabel);
    barsContainer.appendChild(barWrapper);
  });
  
  // Add today's uptime value
  const todayLabel = document.createElement('div');
  todayLabel.className = 'today-label';
  todayLabel.innerHTML = `
    <div>Today</div>
    <div class="value">${formatTime(uptimeHistory[6])}</div>
  `;
  chartContainer.appendChild(todayLabel);
  
  // Add summary
  const totalMinutes = uptimeHistory.reduce((a, b) => a + b, 0);
  const avgMinutes = Math.round(totalMinutes / 7);
  
  const summary = document.createElement('div');
  summary.className = 'summary';
  summary.innerHTML = `
    <div>Total: ${formatTime(totalMinutes)} | Daily Avg: ${formatTime(avgMinutes)}</div>
    <div>Energy saving: <span class="status">${getEnergyStatus(avgMinutes).text}</span></div>
  `;
  summary.querySelector('.status').style.color = getEnergyStatus(avgMinutes).color;
  chartContainer.appendChild(summary);
}

// Format minutes as hours and minutes
function formatTime(minutes) {
  const hours = Math.floor(minutes / 60);
  const mins = minutes % 60;
  
  if (hours === 0) {
    return `${mins}m`;
  } else if (mins === 0) {
    return `${hours}h`;
  } else {
    return `${hours}h ${mins}m`;
  }
}

// Determine energy saving status based on average daily uptime
function getEnergyStatus(avgMinutes) {
  if (avgMinutes < 60) { // Less than 1 hour
    return { text: 'Excellent', color: '#00E676' }; // Bright green
  } else if (avgMinutes < 180) { // Less than 3 hours
    return { text: 'Good', color: '#76FF03' }; // Light green
  } else if (avgMinutes < 360) { // Less than 6 hours
    return { text: 'Fair', color: '#FFEB3B' }; // Yellow
  } else if (avgMinutes < 720) { // Less than 12 hours
    return { text: 'Poor', color: '#FF9800' }; // Orange
  } else {
    return { text: 'Very Poor', color: '#F44336' }; // Red
  }
}

// Add some basic CSS for non-React implementations
function addLightEmissionStyles() {
  const style = document.createElement('style');
  style.textContent = `
    .light-emission-chart {
      font-family: Arial, sans-serif;
      padding: 20px;
      background-color: #2d2d2d;
      border-radius: 8px;
      color: white;
      margin: 20px 0;
    }
    
    .chart-title {
      text-align: center;
      margin-bottom: 20px;
      font-size: 14px;
      font-weight: normal;
      text-transform: uppercase;
      letter-spacing: 1px;
    }
    
    .bars-container {
      display: flex;
      justify-content: space-between;
      align-items: flex-end;
      height: 150px;
      position: relative;
      margin-bottom: 30px;
    }
    
    .bar-wrapper {
      width: 12%;
      position: relative;
      height: 100%;
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    
    .bar {
      width: 100%;
      background-color: #06A7A5;
      border-top-left-radius: 2px;
      border-top-right-radius: 2px;
      position: absolute;
      bottom: 0;
      transition: height 0.3s ease-in-out;
    }
    
    .bar.today {
      background-color: #27c3c3;
    }
    
    .day-label {
      position: absolute;
      bottom: -25px;
      font-size: 12px;
      color: rgba(255, 255, 255, 0.6);
    }
    
    .today-label {
      position: absolute;
      top: 50px;
      right: 20px;
      text-align: right;
      color: rgba(255, 255, 255, 0.6);
      transition: opacity 0.2s ease-in-out;
    }
    
    .today-label div:first-child {
      font-size: 12px;
      margin-bottom: 2px;
    }
    
    .today-label .value {
      font-size: 16px;
      font-weight: 500;
      color: white;
    }
    
    .summary {
      margin-top: 10px;
      font-size: 12px;
      color: rgba(255, 255, 255, 0.6);
    }
    
    .summary .status {
      font-weight: 500;
    }
  `;
  document.head.appendChild(style);
}

// Initialize when DOM is ready
document.addEventListener('DOMContentLoaded', function() {
  addLightEmissionStyles();
  initLightEmissionTracker();
});