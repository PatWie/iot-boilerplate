package api

import (
	"errors"
	"fmt"
	"net/http"

	"github.com/go-chi/jwtauth"
	"github.com/go-chi/render"
)

var MemoryCounter = int64(0)

type Message struct {
	ID   int64  `json:"id"`
	Key  string `json:"key"`
	Info int64  `json:"info"`
}

type MessageRequest struct {
	*Message

	ProtectedID string `json:"id"` // Override 'id' json to have more control.
}

func (a *MessageRequest) Bind(r *http.Request) error {
	if a.Message == nil {
		return errors.New("missing required Message fields.")
	}

	// Just a post-process after a decode.
	a.ProtectedID = "" // Unset the protected ID.
	return nil
}

type MessageResponse struct {
	*Message

	Counter  int64 `json:"counter"`
	DeviceId int64 `json:"device_id"`
}

func (rd *MessageResponse) Render(w http.ResponseWriter, r *http.Request) error {
	rd.Counter = MemoryCounter
	return nil
}

func NewMessageResponse(message *Message) *MessageResponse {
	resp := &MessageResponse{Message: message}
	resp.DeviceId = 0
	MemoryCounter += 1
	return resp
}

// CreateMessage persists the posted Message and returns it
// back to the client as an acknowledgement.
func CreateMessage(w http.ResponseWriter, r *http.Request) {
	data := &MessageRequest{}
	if err := render.Bind(r, data); err != nil {
		render.Render(w, r, ErrInvalidRequest(err))
		return
	}

	message := data.Message
	fmt.Printf("Got message %+v\n", message)
	response := NewMessageResponse(message)

	_, claims, err := jwtauth.FromContext(r.Context())
	if err == nil {
		switch t := claims["device_id"].(type) {

		case float64:
			response.DeviceId = int64(t)
		}

	}

	render.Status(r, http.StatusCreated)
	render.Render(w, r, response)
}
