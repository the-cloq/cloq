(function() {
  'use strict';

  var waves = document.getElementById("tabs");
  var tabPanels = document.querySelectorAll('.tab_panel');
  var tabHeadings = document.querySelectorAll('.tab_heading');
  var tabPanelsArray = [].slice.call(tabPanels);
  var tabHeadingsArray = [].slice.call(tabHeadings);

  function checkTabs(target, arr) {
    arr.forEach(function(currentItem, index, array) {
      if (currentItem !== target) {
        currentItem.setAttribute("aria-hidden", true);
        tabHeadings[index].setAttribute("aria-selected", false);
      } else {
        currentItem.setAttribute("aria-hidden", false);
        tabHeadings[index].setAttribute("aria-selected", true);
      }
    });
  }

  function activateTab(event) {
    var tabTarget = event.target.getAttribute("aria-controls");
    var tabTargetPanel = document.querySelector("#" + tabTarget);

    checkTabs(tabTargetPanel, tabPanelsArray);

    localStorage.setItem('lastTab', tabTarget);
  }

  tabHeadings.forEach(function(currentItem, index, array) {
    currentItem.addEventListener('click', activateTab);
  });

  if (localStorage.length > 0) {
    var lastOpenedTab = document.getElementById(localStorage.getItem('lastTab'));

    checkTabs(lastOpenedTab, tabPanelsArray);
  }

}());