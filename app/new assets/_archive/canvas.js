const btn = document.getElementById('submitBtn');
btn.classList.add('expanded');
let timeoutId;
let lastScroll = window.pageYOffset;
let isExpanded = false;
let isScrollLocked = false;
let scrollUpTimeout = 3000; // ms - adjust as desired

btn.addEventListener('click', () => {
  expandLozenge(3000); // 3 seconds on click
});

window.addEventListener('scroll', () => {
  const currentScroll = window.pageYOffset;
  const scrollingDown = currentScroll > lastScroll;
  lastScroll = currentScroll;

  if (scrollingDown) {
    if (isExpanded) {
      collapseLozenge();
    }
    isScrollLocked = true;
  } else {
    if (isScrollLocked && !isExpanded) {
      expandLozenge(scrollUpTimeout); // short timeout on scroll up
    }
    isScrollLocked = false;
  }
});

function expandLozenge(duration) {
  clearTimeout(timeoutId);
  btn.classList.add('expanded');
  isExpanded = true;
  timeoutId = setTimeout(() => {
    collapseLozenge();
  }, duration);
}

function collapseLozenge() {
  clearTimeout(timeoutId);
  btn.classList.remove('expanded');
  isExpanded = false;
}
