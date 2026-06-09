function focusElement(element) {
    if (element) {
        element.focus({ preventScroll: true });
    }
}

function scrollToEnd() {
    document.getElementById('conversation-bottom')?.scrollIntoView({ behavior: 'smooth', block: 'end' });
}
