document.addEventListener("DOMContentLoaded", () => {
  const grid = document.querySelector(".assets-grid");
  const path = document.querySelector(".assets-path");

  grid.addEventListener("click", (e) => {
    const item = e.target.closest(".asset-item");
    if (!item) return;

    const type = item.dataset.type;
    const name = item.querySelector(".asset-name").textContent;

    if (type === "folder") {
      path.textContent += "/" + name;
      console.log(`Opened folder: ${name}`);
    } else {
      console.log(`Selected asset: ${name}`);
    }
  });

  document.getElementById("refresh-assets")?.addEventListener("click", () => {
    console.log("Assets refreshed");
  });

  document.getElementById("clear-assets")?.addEventListener("click", () => {
    grid.innerHTML = "";
  });
});
