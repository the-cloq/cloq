
var tempSlider = document.getElementById("slider1");
var tempSlider2 = document.getElementById("slider2");
var tempSlider3 = document.getElementById("slider3");
var tempSlider4 = document.getElementById("slider4");
var tempSlider5 = document.getElementById("slider5");

var brightness = document.getElementById("brightness");
var sensor_timer = document.getElementById("sensor_timer");
var summer_offset = document.getElementById("summer_offset");
var winter_offset = document.getElementById("winter_offset");

var cool = document.getElementById("cool");
var warm = document.getElementById('warm');
var hot = document.getElementById('hot');
var inputs = [cool, warm, hot];
var led = [brightness];
var timer = [sensor_timer];
var summOff = [summer_offset];
var wintOff = [winter_offset];


var c2 = brightness.value;
var c3 = sensor_timer.value;
var so = summer_offset.value;
var wo = winter_offset.value;

var c = cool.value;
var w = warm.value;
var h = hot.value;
tmin = 0;
tmax = 38;
slip = 2;

noUiSlider.create(tempSlider, {
   start: [c, w, h],
   connect: [true, true, true, true],
   tooltips: true,
   format: wNumb({
      decimals: 0
   }),
   range: {
      min: tmin,
      max: tmax
   }
});

var connect = tempSlider.querySelectorAll('.noUi-connect');
var classes = ['c-1-color', 'c-2-color', 'c-3-color', 'c-4-color'];
for (var i = 0; i < connect.length; i++) {
   connect[i].classList.add(classes[i]);
}
var blob = tempSlider.querySelectorAll('.noUi-handle');
var classes = ['c-2-color', 'c-3-color', 'c-4-color'];
for (var i = 0; i < blob.length; i++) {
   blob[i].classList.add(classes[i]);
};
tempSlider.noUiSlider.on('change', function (values, handle) {
   if (values[handle] < slip) {
      tempSlider.noUiSlider.set(slip);
   }
});
tempSlider.noUiSlider.on('update', function (values, handle) {
   inputs[handle].value = values[handle];
});



noUiSlider.create(tempSlider2, {
   start: [c2],
    connect: 'lower',
   tooltips: true,
   format: wNumb({
      decimals: 0
   }),
   range: {
        'min': 0,
        'max': 50
   }
});

tempSlider2.noUiSlider.on('change', function (values, handle) {
   if (values[handle] < slip) {
      tempSlider2.noUiSlider.set(slip);
   }
});
tempSlider2.noUiSlider.on('update', function (values, handle) {
   led[handle].value = values[handle];
});



noUiSlider.create(tempSlider3, {
   start: [c3],
    connect: 'lower',
   tooltips: true,
   format: wNumb({
      decimals: 0
   }),
   range: {
        'min': 0,
        'max': 15
   }
});

tempSlider3.noUiSlider.on('change', function (values, handle) {
   if (values[handle] < 1) {
      tempSlider3.noUiSlider.set(1);
   }
});

tempSlider3.noUiSlider.on('update', function (values, handle) {
   timer[handle].value = values[handle];
});



noUiSlider.create(tempSlider4, {
   start: [so],
    connect: 'lower',
   tooltips: true,
   format: wNumb({
      decimals: 0
   }),
   range: {
        'min': -14,
        'max': 12
   }
});

tempSlider4.noUiSlider.on('change', function (values, handle) {
   if (values[handle] < -12) {
      tempSlider4.noUiSlider.set(-12);
   }
});
tempSlider4.noUiSlider.on('update', function (values, handle) {
   summOff[handle].value = values[handle];
});


noUiSlider.create(tempSlider5, {
   start: [wo],
    connect: 'lower',
   tooltips: true,
   format: wNumb({
      decimals: 0
   }),
   range: {
        'min': -14,
        'max': 12
   }
});

tempSlider5.noUiSlider.on('change', function (values, handle) {
   if (values[handle] < -12) {
      tempSlider5.noUiSlider.set(-12);
   }
});
tempSlider5.noUiSlider.on('update', function (values, handle) {
   wintOff[handle].value = values[handle];
});


$(function() {
    $('.radio_container input[type="radio"]').on('change', function(){         
        var $target = $('.radio_container input[type="radio"]:checked');
        $(".showhide").hide();
        $( $target.attr('data-section') ).show();
    }).trigger('change');
  
});

var swiper = new Swiper('.swiper-container', {pagination: '.swiper-pagination',slidesPerView: 1,paginationClickable: true,loop: false,paginationBulletRender: function (index,className) {var tabsName = ['Hours & Minutes', 'Seconds'];if ( index === (tabsName.length - 1) ) {return  '<span class="' + className + '">'+ tabsName[index] + '</span>'+ '<div class="active-mark "></div>';}return '<span class="' + className + '">' + tabsName[index] + '</span>';}});

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

$(function() {
  $('.radio_container input[type="radio"]').on('change', function(){         
      var $target = $('.radio_container input[type="radio"]:checked');
      $(".showhide").hide();
      $( $target.attr('data-section') ).show();
  }).trigger('change');
});

$('.dst-panels').hide();
$("input[id$='auto']").click(function() {
  $('.dst-panels').hide();
});

$("input[id$='summer']").click(function() {
  $('.dst-panels').show();
});

$("input[id$='winter']").click(function() {
  $('.dst-panels').show();
});
