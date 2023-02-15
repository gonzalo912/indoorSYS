const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset='utf-8'>
  <title>RIEGO</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
</head>
<body>
  <a href="/water">CONFIGURAR RIEGO</a><br>
  <a href="/stir">CONFIGURAR REVOLVER</a>
  <a href="/togglePump">ACTIVAR/DESACTIVAR BOMBA DE RIEGO</a>
</body>
</html>
)=====";


const char stir_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>
  <form action="/stir" method="post">
    <label for="mixMins">Cantidad de minutos a revolver:</label>
    <input type="number" name="mixMins">
    <label for="waitToMixMins">Cada cuanto revolver:</label>
    <input type="number" name="waitToMixMins">
    <button type="submit">OK</button>
  </form>
</body>
</html>
)=====";

const char water_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>
  <form action="/water" method="post">
    <label for="waterMins">Cantidad de minutos a regar:</label>
    <input type="number" name="waterMins">
    <button type="submit">OK</button>
  </form>
</body>
</html>
)=====";