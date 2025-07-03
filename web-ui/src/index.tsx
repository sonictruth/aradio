import { createRoot } from 'react-dom/client';
import { App } from './App';
import * as cssText from 'bundle-text:./index.css';

let style = document.createElement('style');
style.textContent = cssText;

document.head.appendChild(style);

interface LinkAttributes {
    rel: string;
    href: string;
    crossOrigin?: string;
}

const appendLink = (attributes: LinkAttributes): void => {
    const link: HTMLLinkElement = document.createElement('link');
    Object.entries(attributes).forEach(([key, value]) => {
        (link as any)[key] = value;
    });
    document.head.appendChild(link);
};


appendLink({ rel: 'preconnect', href: 'https://fonts.googleapis.com' });
appendLink({ rel: 'preconnect', href: 'https://fonts.gstatic.com', crossOrigin: '' });
appendLink({ rel: 'stylesheet', href: 'https://fonts.googleapis.com/css2?family=Bungee+Spice&display=swap' });

let container = document.getElementById("app")!;
let root = createRoot(container)
root.render(
    <App />
);
