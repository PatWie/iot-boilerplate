package auth

import (
	"fmt"

	"github.com/go-chi/jwtauth"
)

func GetTokenAuth() *jwtauth.JWTAuth {

	tokenAuth := jwtauth.New("HS256", []byte("secret"), nil)
	// For debugging/example purposes, we generate and print
	// a sample jwt token with claims `user_id:123` here:
	_, tokenString, _ := tokenAuth.Encode(map[string]interface{}{"device_id": 123})
	fmt.Printf("DEBUG: a sample jwt is %s\n\n", tokenString)

	return tokenAuth
}
