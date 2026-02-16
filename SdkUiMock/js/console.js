document.addEventListener("DOMContentLoaded", () => {
  const consoleOutput = document.querySelector(".console-output");
  const filters = document.querySelectorAll(".filter");
  const clearButton = document.querySelector(".clear");

  filters.forEach(btn => {
    btn.addEventListener("click", () => {
      filters.forEach(b => b.classList.remove("active"));
      btn.classList.add("active");

      const type = btn.dataset.type;
      document.querySelectorAll(".console-line").forEach(line => {
        if (type === "all" || line.classList.contains(type)) {
          line.style.display = "block";
        } else {
          line.style.display = "none";
        }
      });
    });
  });

  clearButton.addEventListener("click", () => {
    consoleOutput.innerHTML = "";
  });
});
