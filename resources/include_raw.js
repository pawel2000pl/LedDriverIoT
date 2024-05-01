"use strict";

class IncludeRaw extends HTMLElement {

    static observedAttributes = ["src"];

    constructor() {
        super();
    }

    attributeChangedCallback(name, oldValue, newValue) { 
        if (name === 'src') {   
            fetch(newValue).then(async (response)=>{
                this.innerHTML = await response.text();
            });
        }
    }

};
customElements.define('include-raw', IncludeRaw);
