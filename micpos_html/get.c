#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	char pszRequest[100]={0};
	char pszResourcePath[]="/micpos.php";
	char pszHostAddress[]="http://localhost";

	printf(pszRequest, "GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\n\r\n", pszResourcePath, pszHostAddress);
	printf("Created Get Request is below: \n\n\n");
	printf("%s", pszRequest);

	return 0;
}
