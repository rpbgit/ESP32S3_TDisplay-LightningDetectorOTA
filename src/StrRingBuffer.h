#ifndef STR_RING_BUFFER_H
#define STR_RING_BUFFER_H 1

#include <cstring> // For strcpy, strcat

constexpr size_t BUFFER_SIZE = 40;    // Number of slots in the buffer
constexpr size_t MAX_STRING_LEN = 200;  // Max length of each string

class StringRingBuffer {
public:
    StringRingBuffer() : head(0), tail(0), full(false) {
        // Initialize the buffer with empty strings
        for (size_t i = 0; i < BUFFER_SIZE; ++i) {
            buffer[i][0] = '\0';
        }
    }

    // Put a string into the ring buffer (overwrites oldest if full)
    void put(const char* str) {
        if (strlen(str) >= MAX_STRING_LEN) {
            // Handle long strings (e.g., truncate or log a warning)
            Serial.println("ERROR- put(), string too long, truncating.");
        }
        strncpy(buffer[head], str, MAX_STRING_LEN - 1);
        buffer[head][MAX_STRING_LEN - 1] = '\0'; // Ensure null-termination

        if (full) {
            tail = (tail + 1) % BUFFER_SIZE; // Move tail forward to overwrite
        }
        head = (head + 1) % BUFFER_SIZE;
        full = head == tail;
//Serial.printf("\nring_put(h:%d, t:%d, f:%d)\n", head, tail, full);
    }

    // Get the string from the tail and move the tail forward
    const char* get() {
        if (isEmpty()) {
            return nullptr;
        }

        const char* result = buffer[tail];
        tail = (tail + 1) % BUFFER_SIZE;
        full = false;
        return result;
    }
    // Get the string from the tail and move the tail forward
    bool getuntested(char* dest, size_t dest_size) {
        if (isEmpty()) {
            return false; // Indicate that the buffer is empty
        }

        // Copy the string from the tail into the destination buffer
        strncpy(dest, buffer[tail], dest_size - 1);
        dest[dest_size - 1] = '\0'; // Ensure null-termination

        tail = (tail + 1) % BUFFER_SIZE;
        full = false;
        return true; // Indicate success
    }

    // Concatenate all strings in the buffer into a single string
    const char* concat_all(char* result)  {
        result[0] = '\0';  // Initialize as empty string
        size_t index = tail;
        while (index != head || (full && index == head)) {
            // Check if the result buffer is large enough, for test ONLY
            // if (strlen(result) + strlen(buffer[index]) + 2 >= result_size) { // +2 for space and null terminator
            //     Serial.println("ERROR- concat_all(), result buffer too small, truncating.");
            //     break; // Prevent overflow
            // }
            strcat(result, buffer[index]);
            strcat(result, " "); // Add space between strings
            index = (index + 1) % BUFFER_SIZE;
            //if (!full && index == head) break;
            if(index == head) break;
        }
        //Serial.printf("ring_concat_END(h:%d, t:%d, f:%d)", head, tail, full);
        
        return result;
    }
    
    // Concatenate all strings in the buffer into a single string and remove them from the buffer
    // This function will clear the buffer after concatenation
    const char* concat_and_remove_all(char* result)
    {
        result[0] = '\0'; // Initialize as empty string
        
//Serial.printf("\nring_concatBEGIN(h:%d, t:%d, f:%d)\n", head, tail, full);
        while (!isEmpty()) {
            strcat(result, buffer[tail]); // Append the current string
            strcat(result, " "); // Add a space between strings
            tail = (tail + 1) % BUFFER_SIZE; // Move the tail forward
            full = false; // Buffer is no longer full
        }
        
        // Remove trailing space, if any
        size_t len = strlen(result);
        if (len > 0 && result[len - 1] == ' ') {
            result[len - 1] = '\0';
        }
        
//Serial.printf("\nring_concatEND(h:%d, t:%d, f:%d)\n", head, tail, full);
        return result;
    }
    
    // Concatenate all strings in the buffer into a single string
    const char* concat_alluntested(char* result, size_t result_size) {
        result[0] = '\0';  // Initialize as an empty string
        char* current_pos = result; // Pointer to track the current position in the result buffer
        size_t remaining_size = result_size; // Track remaining space in the result buffer

        size_t index = tail;
        while (index != head || (full && index == head)) {
            size_t str_len = strlen(buffer[index]);

            // Check if there is enough space for the string and a space
            if (remaining_size <= str_len + 1) { // +1 for space or null terminator
                Serial.println("ERROR- concat_all(), result buffer too small, truncating.");
                break; // Prevent overflow
            }

            // Copy the string into the result buffer
            strncpy(current_pos, buffer[index], remaining_size - 1);
            current_pos += str_len;
            remaining_size -= str_len;

            // Add a space if there is still room
            if (remaining_size > 1) {
                *current_pos = ' ';
                current_pos++;
                remaining_size--;
            }

            index = (index + 1) % BUFFER_SIZE;
            if (index == head && !full) break;
        }

        // Remove the trailing space, if any
        if (current_pos > result && *(current_pos - 1) == ' ') {
            *(current_pos - 1) = '\0';
        } else {
            *current_pos = '\0'; // Null-terminate the result
        }

        return result;
    }
    // Delete all strings in the buffer
    void delete_all() {
        head = tail;
        full = false;
        // Optionally, clear the buffer's content
        for (size_t i = 0; i < BUFFER_SIZE; ++i) {
            buffer[i][0] = '\0';
        }
    }

    bool isEmpty() const {
        return !full && head == tail;
        //return head == tail && !full;
    }

private:
    char buffer[BUFFER_SIZE][MAX_STRING_LEN]; // Statically allocated buffer
    size_t head, tail;
    bool full;
};
#endif

#if 0
//////////////////////////////////////////////////////////////////////////
int main() {
	StringRingBuffer ring;
	char catbuf[500] = {0};

	ring.put(":one");
	ring.put(":two");
	ring.put(":three");

	ring.concat_all(catbuf);
	std::cout << "concat after 3: " << catbuf << std::endl;
	*catbuf = 0;

	ring.delete_all();
	ring.put(":four");
	ring.put(":five_max");
	ring.put(":six");

	ring.concat_all(catbuf);
	std::cout << "concat after delete and add 4,5,6: " << catbuf << std::endl;
	*catbuf = 0;

	ring.put(":seven");
	while (!ring.isEmpty()) {
		cout << "Get: " << ring.get() << std::endl;
	}
	ring.concat_all(catbuf);
	std::cout << "concat after adding 7 and get all - expect null : " << catbuf << std::endl;
	*catbuf = 0;

	while (!ring.isEmpty()) {
		std::cout << "Get: " << ring.get() << std::endl;
	}

	ring.put("eight");
	ring.concat_all(catbuf);
	std::cout << "Concatenated 3 - expect 8 : " << catbuf << std::endl;
	*catbuf = 0;

	ring.delete_all();

	return 0;
}

#endif