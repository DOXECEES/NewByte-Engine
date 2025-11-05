// // js/loadComponents.js

// /**
//  * Загружает HTML компонент в элемент с указанным id
//  * @param {string} elementId - id элемента в index.html
//  * @param {string} url - путь к HTML-файлу компонента
//  */
// async function loadComponent(elementId, url) {
//   try {
//     const response = await fetch(url);
//     if (!response.ok) throw new Error(`Ошибка загрузки ${url}: ${response.status}`);
//     const html = await response.text();
//     const container = document.getElementById(elementId);
//     if (!container) throw new Error(`Элемент с id="${elementId}" не найден`);
//     container.innerHTML = html;
//   } catch (err) {
//     console.error(err);
//   }
// }

// // Загружаем все компоненты
// document.addEventListener("DOMContentLoaded", () => {
//   loadComponent("inspector", "components/inspector.html");
//   loadComponent("hierarchy", "components/hierarchy.html");
//   loadComponent("console", "components/console.html");
//   loadComponent("scene", "components/scene.html");
// });
