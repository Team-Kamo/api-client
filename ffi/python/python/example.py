from api import *

if __name__ == "__main__":
    print("**Example Code**")
    print(
        "Note: This is just an example, so there is no exception handling or anything like that."
    )
    with Api("mock", "http://localhost:3000", "/api/v1") as api:
        if input("Do you want to connect to an existing room? (Y/N) ") != "Y":
            roomName = input("Create a room. Enter a room name: ")
            roomId = api.create_room(roomName)
            print(f"Room (id = {roomId}) has been created.")
        else:
            roomId = int(input("Enter a room id: "))

        api.connect_room(roomId)
        print("Connected to room.")

        while True:
            command = input("Command: ")
            if command == "help":
                print("upload:   Upload text.")
                print("download: Download text.")
                print("quit:     Exit.")
                print("other:    Ask Soon:)")
            elif command == "upload":
                text = input("Enter: ")
                api.upload_text_content(text)
            elif command == "download":
                content = api.get_content()
                print(content.data.clipboard.decode("utf-8"))
            elif command == "quit":
                print("Bye!")
                break
            else:
                print("I'm sure Soon can answer your questions!")
