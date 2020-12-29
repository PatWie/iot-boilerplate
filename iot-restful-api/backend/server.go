package main

import (
	"flag"
	"fmt"
	"log"
	"net/http"

	"iot-restful-api/api"
	"iot-restful-api/auth"

	"github.com/go-chi/chi"
	"github.com/go-chi/chi/middleware"
	"github.com/go-chi/jwtauth"
	"github.com/go-chi/render"
)

func iotRouter() chi.Router {
	r := chi.NewRouter()

	r.Group(func(r chi.Router) {

		// Seek, verify and validate JWT tokens
		r.Use(jwtauth.Verifier(auth.GetTokenAuth()))

		// Handle valid / invalid tokens. In this example, we use
		// the provided authenticator middleware, but you can write your
		// own very easily, look at the Authenticator method in jwtauth.go
		// and tweak it, its not scary.
		r.Use(jwtauth.Authenticator)

		// $ curl -X POST -H"Authorization: BEARER eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJkZXZpY2VfaWQiOjEyM30.cgkvt-XXIl85Bi4NSqGS0YD-u_4gPkZkirRqNggJMCo" -d '{"key":"value", "info": 1}' http://localhost:3333/v1/iot/secure_messages
		r.Route("/secure_messages", func(r chi.Router) {
			r.Post("/", api.CreateMessage) // POST /messages

		})
	})

	// $ curl -X POST -d '{"key":"value", "info": 1}' http://localhost:3333/v1/iot/messages
	r.Route("/messages", func(r chi.Router) {
		r.Post("/", api.CreateMessage) // POST /messages

	})

	return r
}

func createRouter() *chi.Mux {
	r := chi.NewRouter()

	r.Use(middleware.RequestID)
	r.Use(middleware.Logger)
	r.Use(middleware.Recoverer)
	r.Use(middleware.RequestID)
	r.Use(middleware.URLFormat)
	r.Use(render.SetContentType(render.ContentTypeJSON))

	r.Get("/", func(w http.ResponseWriter, r *http.Request) {
		w.Write([]byte(""))
	})

	r.Get("/ping", func(w http.ResponseWriter, r *http.Request) {
		w.Write([]byte("pong"))
	})

	r.Route("/v1", func(r chi.Router) {
		r.Mount("/iot", iotRouter())
	})

	return r
}

func main() {

	port := flag.Int("port", 3333, "port to serve")

	flag.Parse()

	r := createRouter()

	fmt.Printf("Serve at :%v\n", *port)
	log.Fatal(http.ListenAndServe(fmt.Sprintf(":%v", *port), r))
}
