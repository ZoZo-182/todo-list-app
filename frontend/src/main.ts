import './style.css'

    const login = document.getElementById('loginForm')!;
    const register = document.getElementById('registerForm')!;

    let isLogin = true;

    function render() {
        login.classList.toggle("hidden", !isLogin);
        register.classList.toggle("hidden", isLogin);
    }

    document.getElementById('btn-login')!.addEventListener("click", () => {
        isLogin = true;
        render();
    });
    document.getElementById('btn-register')!.addEventListener("click", () => {
        isLogin = false;
        render();
    });

    render();
