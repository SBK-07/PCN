from google import genai
client = genai.Client()
chat = client.chats.create(model="gemini-3.5-flash")

print("\n🤖 Gemini Terminal Chat")
print("Type 'exit' to quit or 'reset' to restart.\n")

while True:
    try:
        prompt = input("You: ").strip()

        if not prompt:
            continue

        if prompt.lower() in ("exit", "quit"):
            print("Gemini: See you, bro!")
            break

        if prompt.lower() == "reset":
            chat = client.chats.create(model="gemini-3.5-flash")
            print("Gemini: Conversation reset!\n")
            continue

        response = chat.send_message(prompt)
        print(f"\nGemini: {response.text}\n")

    except KeyboardInterrupt:
        print("\nChat ended.")
        break
    except Exception as error:
        print(f"\nError: {error}\n")
