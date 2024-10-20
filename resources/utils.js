"use strict";


Array.from(document.getElementsByClassName('link-button')).forEach((button)=>{
    const linkAttr = button.attributes['link'] ?? undefined;
    const link = (linkAttr === undefined) ? './#' : linkAttr.value;
    button.addEventListener('mouseup', e => {
        if (e.button === 0) {
            window.location = link;
        }
    });
    button.addEventListener('mousedown', e => {
        if (e.button === 1) {
            e.preventDefault();
            window.open(link, '_blank');
        }
    });
    button.touchTimer = -1;
    button.addEventListener('touchstart', e => {
        e.preventDefault();
        button.touchTimer = setTimeout(() => {
            window.open(link, '_blank');
        }, 500);
    });
    button.addEventListener('touchend', e => {
        clearTimeout(button.touchTimer);
        button.touchTimer = -1;
    });
});


const $id = (id)=>document.getElementById(id);
const $new = (...args)=>document.createElement(...args);

