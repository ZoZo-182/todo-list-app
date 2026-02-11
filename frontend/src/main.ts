import './style.css'
import typescriptLogo from './typescript.svg'
import viteLogo from '/vite.svg'
//import { setupCounter } from './counter.ts'

document.querySelector<HTMLDivElement>('#app')!.innerHTML = `
  <!-- <div>
    <a href="https://vite.dev" target="_blank">
      <img src="${viteLogo}" class="logo" alt="Vite logo" />
    </a>
    <a href="https://www.typescriptlang.org/" target="_blank">
      <img src="${typescriptLogo}" class="logo vanilla" alt="TypeScript logo" />
    </a>
    <h1>Vite + TypeScript</h1>
    <div class="card">
      <button id="counter" type="button"></button>
    </div>
    <p class="read-the-docs">
      Click on the Vite and TypeScript logos to learn more
    </p>
  </div> -->
  <div class="cred-form">
    <form action="http://localhost:8080/register" method="post">
      <label for="fname">First name:</label>
      <input type="text" id="fname" name="first_name"><br><br>
      <label for="lname">Last name:</label>
      <input type="text" id="lname" name="last_name"><br><br>
      <label for="email">Email:</label>
      <input type="text" id="email" name="email"><br><br>
      <label for="pwd">Password:</label>
      <input type="text" id="pwd" name="password"><br><br>
      <button type="submit">Sumbit</button> 
    </form> 
  </div>
`

// setupCounter(document.querySelector<HTMLButtonElement>('#counter')!)
