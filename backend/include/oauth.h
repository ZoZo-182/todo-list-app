/* after the user clicks the login button, a get request will be sent to the
 * backend. The backend will then return an auth URL that needs to be built
 * based off the callback (redirect) url for that provider.
 * The user will then be redirected to that url
 */
char *build_github_auth_url(void);
char *build_google_auth_url(void);

/* provider redirects back to my app with a code to the frontend which sends
 * it to the backend to exchange the code for access token. does this by
 * sending post req to provider with the access token and the body of the 
 * token contains the client id, secret, code and redirect url
 */
char *exchange_code_for_access_token(const char *code, const char *provider );

/* backend does a get req to provider to get the user's info. JWT is
 * created and it is returned to frontend
 */
char *get_user_info_from_provider(const char *access_token, const char *provider);

