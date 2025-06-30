import { createRoot } from 'react-dom/client';
import { App } from './App';
import * as cssText from 'bundle-text:./index.css';

let style = document.createElement('style');
style.textContent = cssText;
document.head.appendChild(style);

let container = document.getElementById("app")!;
let root = createRoot(container)
root.render(
    <App />
);
