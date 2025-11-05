document.addEventListener("DOMContentLoaded", () => {
  const hierarchy = document.querySelector("#hierarchy");
  if (!hierarchy) return;

  // Добавляем стрелку только тем, у кого есть дети
  hierarchy.querySelectorAll(".hierarchy-item").forEach(item => {
    const children = item.querySelector(".hierarchy-children");
    if (children) {
      const triangle = document.createElement("span");
      triangle.className = "triangle";
      triangle.textContent = item.classList.contains("expanded") ? "▼" : "▶";
      item.prepend(triangle);
    }
  });

  hierarchy.addEventListener("click", (e) => {
    const item = e.target.closest(".hierarchy-item");
    if (!item) return;

    // Клик по стрелке — сворачиваем / разворачиваем
    if (e.target.classList.contains("triangle")) {
      const children = item.querySelector(".hierarchy-children");
      const triangle = e.target;

      if (children) {
        const expanded = item.classList.toggle("expanded");
        children.style.display = expanded ? "flex" : "none";
        triangle.textContent = expanded ? "▼" : "▶";
      }
      return;
    }

    // Клик по элементу — выделение
    hierarchy.querySelectorAll(".hierarchy-item").forEach(i => i.classList.remove("selected"));
    item.classList.add("selected");
  });
});
