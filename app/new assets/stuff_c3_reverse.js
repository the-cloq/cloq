let currentChartMode = localStorage.getItem('viewMode') || "time";
document.querySelectorAll('input[name="view-mode"]').forEach(radio => {
  if (radio.value === currentChartMode) {
    radio.checked = true;
  }
  radio.addEventListener('change', () => {
    // Update the current chart mode based on the selected radio button
    currentChartMode = document.querySelector('input[name="view-mode"]:checked').value;
    localStorage.setItem('viewMode', currentChartMode);
    fetchUptimeData();
  });
});
function updateChart(data) {
    const container = document.querySelector('.bar-container');
    const trackWrapper = container.querySelector('.bar-track-wrapper');
    trackWrapper.innerHTML = '';
  
    const touchLine = document.querySelector('.touch-line');
    const floatingLabel = document.querySelector('.floating-uptime-label');
    const staticLabel = document.querySelector('.static-uptime-label');
    const staticTitle = staticLabel.querySelector('.uptime-title');
    staticTitle.textContent = "Today";
    const uptimeTitle = floatingLabel.querySelector('.uptime-title');
    const uptimeValue = floatingLabel.querySelector('.uptime-value');
    const staticValue = staticLabel.querySelector('.uptime-value');
    const summaryUptime = document.querySelector('.summary-uptime');
    const statusText = document.getElementById('status-text');
    const wrapper = document.querySelector('.bar-chart-wrapper');
  
    const maxValue = Math.max(...data.days, 1);
    const scaleMax = maxValue * 1.4;
    const days = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
    const today = new Date();
    const labels = [];
  
    const todayVal = data.days[6];
    const hours = Math.floor(todayVal / 60);
    const minutes = todayVal % 60;
    const kWh = (todayVal / 60) * (3.75 / 1000);
  
    const priceInput = document.getElementById("priceperkwh");
    const userPrice = parseFloat(priceInput?.value || 0.27);
  
    staticValue.textContent = currentChartMode === "time"
      ? `${hours}h ${minutes}m`
      : formatCost(kWh * userPrice);
  
    for (let i = 6; i >= 0; i--) {
      const d = new Date();
      d.setDate(today.getDate() - i);
      labels.push(days[d.getDay()]);
    }
  
    data.days.forEach((val, i) => {
      const bar = document.createElement('div');
      bar.className = 'bar';
      bar.style.height = '0%';
      bar.style.transition = 'height 0.3s cubic-bezier(0.34, 1.56, 0.64, 1)';
      bar.dataset.index = i;
      bar.dataset.uptime = val;
      bar.style.backgroundColor = (val === 0) ? '#4a558e' : (i === 6) ? '#7349ff' : '#00ffff';
      const label = document.createElement('div');
      label.className = 'bar-label';
      label.textContent = labels[i];
      bar.appendChild(label);
      trackWrapper.appendChild(bar);
  
      setTimeout(() => {
        const percentHeight = Math.max(1, (val / scaleMax) * 100);
        const bounceHeight = percentHeight * 1.05;
  
        bar.style.height = `${bounceHeight}%`;
  
        setTimeout(() => {
          bar.style.transition = 'height 0.3s cubic-bezier(0.34, 1.56, 0.64, 1)';
          bar.style.height = `${percentHeight}%`;
        }, 300);
      }, 30 + i * 100);
    });

    const dividerDelay = 400 + 7 * 100;
    setTimeout(() => requestAnimationFrame(insertWeekDivider), dividerDelay);
  
    const thisWeek = Number(data.thisWeek) || 0;
    const lastWeek = Number(data.lastWeek) || 0;
    //const avg = thisWeek / 7;
    const last7 = data.days.slice(-7); // get last 7 days
    const avg = last7.reduce((sum, val) => sum + val, 0) / last7.length;

  
    const thisWeekKWh = (thisWeek / 60) * (3.75 / 1000);
    const avgKWh = (avg / 60) * (3.75 / 1000);
    const lastWeekKWh = (lastWeek / 60) * (3.75 / 1000);
  
    function formatMinutes(mins) {
      mins = Math.round(mins);
      const h = Math.floor(mins / 60);
      const m = mins % 60;
      return h > 0 ? `${h}h ${m}m` : `${m}m`;
    }
  
    summaryUptime.innerHTML = currentChartMode === "time"
      ? `
        <div class="uptime-right"><span>Last Week</span><br>${formatMinutes(lastWeek)}</div>
        <div class="uptime-left"><span>This Week</span><br>${formatMinutes(thisWeek)}</div>
        <div class="uptime-center"><span>Daily Avg</span><br>${formatMinutes(avg)}</div>
      `
      : `
        <div class="uptime-right"><span>Last Week</span><br>${formatCost(lastWeekKWh * userPrice)}</div>
        <div class="uptime-left"><span>This Week</span><br>${formatCost(thisWeekKWh * userPrice)}</div>
        <div class="uptime-center"><span>Daily Avg</span><br>${formatCost(avgKWh * userPrice)}</div>
      `;
  
    if (data.weeklyMsg) {
      const toast = document.getElementById('weekly-toast');
      if (toast) {
        toast.textContent = data.weeklyMsg;
        toast.style.display = 'block';
        toast.style.opacity = '1';
        setTimeout(() => {
          toast.style.opacity = '0';
          setTimeout(() => {
            toast.style.display = 'none';
          }, 600);
        }, 4000);
      }
    }
  
    statusText.innerHTML = "<span>Energy Saving:</span> " + (avg <= 180 ? "Excellent" : avg <= 360 ? "Good" : "Bad");
    statusText.style.color = avg <= 180 ? "#FFFFFF" : avg <= 360 ? "#FFD700" : "#FF0000";

  function updateTouchLine(x) {
    const bars = Array.from(container.querySelectorAll('.bar'));
    const rect = container.getBoundingClientRect();
    const margin = 5;
    const relX = Math.max(margin, Math.min(x - rect.left, container.offsetWidth - margin));

    touchLine.style.left = `${relX}px`;
    touchLine.style.display = 'block';

    const labelW = floatingLabel.offsetWidth;
    let labelX = Math.max(0, Math.min(relX - labelW / 2, wrapper.offsetWidth - labelW));
    floatingLabel.style.left = `${labelX}px`;
    floatingLabel.style.display = 'flex';
    staticLabel.style.opacity = '0';

    let closest = null, minDist = Infinity;
    bars.forEach(bar => {
      const barX = bar.getBoundingClientRect().left + bar.offsetWidth / 2;
      const dist = Math.abs(barX - x);
      if (dist < minDist) {
        minDist = dist;
        closest = bar;
      }
    });

    if (closest) {
      const up = parseInt(closest.dataset.uptime, 10);
      const idx = parseInt(closest.dataset.index, 10);
      uptimeTitle.textContent = idx === 6 ? "Today" : labels[idx];
      const cost = (up / 60) * (3.75 / 1000) * userPrice;
      uptimeValue.textContent = currentChartMode === "time"
  ? `${Math.floor(up/60)}h ${up%60}m`
  : formatCost(cost);
      
      // HAPTIC or SOUND feedback
      if (idx !== lastTouchedIndex) {
          if (navigator.vibrate) navigator.vibrate(10); 
          lastTouchedIndex = idx;
      }
    }
  }

  container.addEventListener('touchstart', e => {
      startX = e.touches[0].clientX;
      startY = e.touches[0].clientY;
      dragStarted = false;
      isDragging = false;
    }, { passive: true });
    
    container.addEventListener('touchmove', e => {
      const dx = e.touches[0].clientX - startX;
      const dy = e.touches[0].clientY - startY;
    
      if (!dragStarted) {
        if (Math.abs(dy) > Math.abs(dx)) {
          // Vertical scroll detected — do nothing, allow scroll
          isDragging = false;
          return;
        } else {
          // Horizontal drag detected
          isDragging = true;
          dragStarted = true;
        }
      }

      if (isDragging) {
        e.preventDefault(); // Only block scrolling if dragging horizontally
        updateTouchLine(e.touches[0].clientX);
      }
    }, { passive: false });
    
    container.addEventListener('touchend', () => {
      isDragging = false;
      dragStarted = false;
      floatingLabel.style.display = 'none';
      staticLabel.style.opacity = '1';
      touchLine.style.display = 'none';
    });

    container.addEventListener('mousemove', e => updateTouchLine(e.clientX));
      container.addEventListener('mouseleave', () => {
      floatingLabel.style.display = 'none';
      staticLabel.style.opacity = '1';
      touchLine.style.display = 'none';
  });
}

function insertWeekDivider() {
    const container = document.querySelector('.bar-container');
    const bars = container.querySelectorAll('.bar');
    const todayJSIndex = new Date().getDay(); // 0 = Sunday
    const sundayIndex = (6 - todayJSIndex + 7) % 7;
  
    // Remove existing divider(s)
    container.querySelectorAll('.week-divider').forEach(div => div.remove());
  
    if (bars[sundayIndex]) {
      const divider = document.createElement('div');
      divider.className = 'week-divider';
      const left = Math.max(0, bars[sundayIndex].offsetLeft - 5);
      divider.style.left = `${left}px`;
      container.appendChild(divider);
    }
  }

function formatCost(cost) {
    const pennies = (cost * 100).toFixed(1); // Convert to pennies and round to 1 decimal place
    return `${pennies}p`; // Return in "pennies" format
  }

function fetchUptimeData() {
  const spinner = document.getElementById('chart-spinner');
  spinner.style.display = 'block'; // Show spinner before fetching

  fetch('/uptime-data')
    .then(res => res.json())
    .then(data => {
      updateChart(data);
    })
    .finally(() => {
      spinner.style.display = 'none'; // Hide spinner after chart updates
    });
}

document.addEventListener('DOMContentLoaded', () => {
  fetchUptimeData();
  setInterval(fetchUptimeData, 60000);
});

function resetUptime() {
  fetch('/reset-uptime', { method: 'POST' })
    .then(() => {
      fetchUptimeData();
      alert("Uptime history has been reset!");
    })
    .catch(err => {
      console.error('Reset failed:', err);
      alert("Failed to reset uptime.");
    });
}

document.addEventListener('DOMContentLoaded', () => {
    const chartPanel = document.getElementById('panel5');
  
    if (!chartPanel) return; // safety check
  
    const observer = new MutationObserver(() => {
      if (chartPanel.getAttribute('aria-hidden') === 'false') {
        insertWeekDivider(); // your divider placement logic
      }
    });
  
    observer.observe(chartPanel, { attributes: true });
  });

document.getElementById("freshRolloverBtn").addEventListener("click", () => {
  const status = document.getElementById("rolloverStatus");
  status.textContent = "⏳ Triggering...";
  fetch("/force-rollover", { method: "POST" })
    .then(() => {
      status.textContent = "✅ Rollover complete";
      setTimeout(() => (status.textContent = ""), 3000);
    })
    .catch(err => {
      console.error("❌ Rollover failed:", err);
      status.textContent = "❌ Rollover failed";
    });
});