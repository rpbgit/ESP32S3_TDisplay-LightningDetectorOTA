#ifndef STATIC_RING_BUFFER_H
#define STATIC_RING_BUFFER_H

#include <Arduino.h> // For Serial
#include <cstring>
#include <cstdio>
#include <cstddef>

static char tempBuffer[64];

template <size_t BUFFER_SIZE, size_t STRING_LENGTH>
class StaticRingBuffer {
    char buffer[BUFFER_SIZE][STRING_LENGTH] = {};
    size_t head = 0;
    size_t count = 0;
    mutable size_t readIndex = 0;

public:
    /**
     * @brief Adds a string to the ring buffer. Overwrites the oldest entry if the buffer is full.
     * @note This is a **destructive** operation if the buffer is full, as it overwrites the oldest element.
     * @param str The string to add to the buffer.
     */
    void put(const char* str) {
        std::snprintf(buffer[head], STRING_LENGTH, "%s", str);
        head = (head + 1) % BUFFER_SIZE;
        if (count < BUFFER_SIZE) ++count;
    }

    /**
     * @brief Retrieves a string at a specific index relative to the oldest entry.
     * @note This is a **non-destructive** operation.
     * @param index The index of the string to retrieve (0 is the oldest).
     * @param outStr The buffer to store the retrieved string.
     * @param outSize The size of the output buffer.
     * @return True if the string was successfully retrieved, false otherwise.
     */
    bool get(size_t index, char* outStr, size_t outSize) const {
        if (index >= count || outSize == 0) return false;
        size_t pos = (head + BUFFER_SIZE - count + index) % BUFFER_SIZE;
        std::memcpy(outStr, buffer[pos], std::min(outSize - 1, STRING_LENGTH));
        outStr[outSize - 1] = '\0';
        return true;
    }
    
    /**
     * @brief Retrieves and removes the oldest string in the buffer.
     * @note This is a **destructive** operation.
     * @param outStr The buffer to store the retrieved string.
     * @param outSize The size of the output buffer.
     * @return True if the string was successfully retrieved and removed, false otherwise.
     */
    bool getAndRemove(char* outStr, size_t outSize) {
        if (count == 0 || outSize == 0) return false;
        size_t oldestIndex = (head + BUFFER_SIZE - count) % BUFFER_SIZE;
        std::memcpy(outStr, buffer[oldestIndex], std::min(outSize - 1, STRING_LENGTH));
        outStr[outSize - 1] = '\0'; // Ensure null termination
        --count; // Remove the oldest element
        return true;
    }
    /**
     * @brief Concatenates all strings in the buffer into a single string.
     * @note This is a **non-destructive** operation.
     * @param outStr The buffer to store the concatenated result.
     * @param outSize The size of the output buffer.
     */
    void concatenate(char* outStr, size_t outSize) const {
        if (outSize == 0) return;
        char* current = outStr;
        size_t remaining = outSize;

        for (size_t i = 0; i < count; ++i) {
            size_t pos = (head + BUFFER_SIZE - count + i) % BUFFER_SIZE;
            size_t len = std::strlen(buffer[pos]);

            if (remaining <= len) break; // Prevent overflow
            std::memcpy(current, buffer[pos], len);
            current += len;
            remaining -= len;

            if (remaining > 1) {
                *current = ' ';
                current++;
                remaining--;
            }
        }

        if (current > outStr && *(current - 1) == ' ') {
            *(current - 1) = '\0'; // Remove trailing space
        } else {
            *current = '\0'; // Null-terminate
        }
    }

    /**
     * @brief Concatenates all strings in the buffer with a separator between them.
     * @note This is a **non-destructive** operation.
     * @param outStr The buffer to store the concatenated result.
     * @param outSize The size of the output buffer.
     * @param sep The separator string to insert between entries.
     */
    void concatenateWithSeparator(char* outStr, size_t outSize, const char* sep) const {
        if (outSize == 0) return;
        outStr[0] = '\0';
        for (size_t i = 0; i < count; ++i) {
            size_t pos = (head + BUFFER_SIZE - count + i) % BUFFER_SIZE;
            std::strncat(outStr, buffer[pos], outSize - std::strlen(outStr) - 1);
            if (i < count - 1) {
                std::strncat(outStr, sep, outSize - std::strlen(outStr) - 1);
            }
        }
    }

    /**
     * @brief Concatenates all strings in the buffer into a single string and removes them from the buffer.
     * @note This is a **destructive** operation. It clears the buffer after concatenation.
     * @param outStr The buffer to store the concatenated result.
     * @param outSize The size of the output buffer.
     */
    void concatenateAndRemove(char* outStr, size_t outSize) {
        if (outSize == 0) return;
        outStr[0] = '\0';

        while (count > 0) {
            size_t pos = (head + BUFFER_SIZE - count) % BUFFER_SIZE;
            std::strncat(outStr, buffer[pos], outSize - std::strlen(outStr) - 1);
            --count; // Remove the oldest element
        }
    }

    /**
     * @brief Concatenates all strings in the buffer with a separator between them and removes them from the buffer.
     * @note This is a **destructive** operation. It clears the buffer after concatenation.
     * @param outStr The buffer to store the concatenated result.
     * @param outSize The size of the output buffer.
     * @param sep The separator string to insert between entries.
     */
    void concatenateWithSeparatorAndRemove(char* outStr, size_t outSize, const char* sep) {
        if (outSize == 0) return;
        outStr[0] = '\0';

        while (count > 0) {
            size_t pos = (head + BUFFER_SIZE - count) % BUFFER_SIZE;
            std::strncat(outStr, buffer[pos], outSize - std::strlen(outStr) - 1);
            --count; // Remove the oldest element

            // Add separator if there are more elements
            if (count > 0) {
                std::strncat(outStr, sep, outSize - std::strlen(outStr) - 1);
            }
        }
    }

    /**
     * @brief Resets the read index used by getNext().
     * @note This is a **non-destructive** operation.
     */
    void resetRead() const { readIndex = 0; }

    /**
     * @brief Retrieves the next string in the buffer sequentially, starting from the oldest.
     * @note This is a **non-destructive** operation.
     * @param outStr The buffer to store the retrieved string.
     * @param outSize The size of the output buffer.
     * @return True if the string was successfully retrieved, false otherwise.
     */
    bool getNext(char* outStr, size_t outSize) const {
        if (readIndex >= count || outSize == 0) return false;
        size_t pos = (head + BUFFER_SIZE - count + readIndex) % BUFFER_SIZE;
        std::memcpy(outStr, buffer[pos], std::min(outSize - 1, STRING_LENGTH));
        outStr[outSize - 1] = '\0';
        ++readIndex;
        return true;
    }

    /**
     * @brief Checks if the buffer is full.
     * @note This is a **non-destructive** operation.
     * @return True if the buffer is full, false otherwise.
     */
    bool isFull() const { return count == BUFFER_SIZE; }

    /**
     * @brief Checks if the buffer is empty.
     * @note This is a **non-destructive** operation.
     * @return True if the buffer is empty, false otherwise.
     */
    bool isEmpty() const { return count == 0; }

    /**
     * @brief Gets the number of elements currently in the buffer.
     * @note This is a **non-destructive** operation.
     * @return The number of elements in the buffer.
     */
    size_t size() const { return count; }

    /**
     * @brief Gets the maximum capacity of the buffer.
     * @note This is a **non-destructive** operation.
     * @return The maximum number of elements the buffer can hold.
     */
    size_t capacity() const { return BUFFER_SIZE; }

    /**
     * @brief Clears all elements from the buffer.
     * @note This is a **destructive** operation. It removes all elements from the buffer.
     */
    void clear() {
        head = 0;
        count = 0;
        readIndex = 0;
    }

    /**
     * @brief Prints the contents of the buffer to the Serial monitor for debugging.
     * @note This is a **non-destructive** operation.
     */
    void debugPrint() const {
        printf("Buffer contents (%zu entries):\n", count);
        for (size_t i = 0; i < count; ++i) {
            size_t pos = (head + BUFFER_SIZE - count + i) % BUFFER_SIZE;
            Serial.printf(" [%zu] %s\n", i, buffer[pos]);
        }
    }
};

#endif // STATIC_RING_BUFFER_H

/* Example usage of StaticRingBuffer
#include <Arduino.h>  
#include "StaticRingBuffer.h"

// Create a ring buffer with 8 entries, each up to 64 characters long
StaticRingBuffer<8, 64> ringBuffer;

void setup() {
    Serial.begin(115200);

    // Add some strings to the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");
    ringBuffer.put("Third");

    // Retrieve and remove the oldest string
    char result[64];
    if (ringBuffer.getAndRemove(result, sizeof(result))) {
        Serial.printf("Retrieved and removed: %s\n", result);
    }

    // Print the remaining buffer contents
    ringBuffer.debugPrint();

    // Concatenate all remaining strings
    char concatenated[256];
    ringBuffer.concatenate(concatenated, sizeof(concatenated));
    Serial.printf("Concatenated: %s\n", concatenated);
}

void loop() {
    // Nothing to do here
}
Here is the **complete test suite** for the `StaticRingBuffer` class, incorporating all the previously discussed tests, including verification of added and extracted strings, and verification of overwrite behavior when the buffer is full.
copy/paste this code into the online compiler at https://www.onlinegdb.com/online_c_compiler#
---
//* Example usage of StaticRingBuffer
//#include <Arduino.h>  
//#include "StaticRingBuffer.h"


void    setup();
void    loop();
int main()
{
    setup();
    loop();
    //exit(0);
}
#define test2 // or test1
#ifdef test1
// Create a ring buffer with 8 entries, each up to 64 characters long
StaticRingBuffer<8, 64> ringBuffer;
void setup() {
    //Serial.begin(115200);

    // Add some strings to the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");
    ringBuffer.put("Third");

    // Retrieve and remove the oldest string
    char result[64];
    if (ringBuffer.getAndRemove(result, sizeof(result))) {
        printf("Retrieved and removed: %s\n", result);
    }

    // Print the remaining buffer contents
    ringBuffer.debugPrint();

    // Concatenate all remaining strings
    char concatenated[256];
    ringBuffer.concatenate(concatenated, sizeof(concatenated));
    printf("Concatenated: %s\n", concatenated);
}

void loop() {
    // Nothing to do here
}
#endif

#ifdef test2

// Create a ring buffer with 4 entries, each up to 64 characters long
StaticRingBuffer<4, 64> ringBuffer;

void testPutAndGet() {
    printf("Running testPutAndGet...\n");

    ringBuffer.clear();

    // Add strings to the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");
    ringBuffer.put("Third");

    // Retrieve strings by index and verify
    char result[64];
    bool success = ringBuffer.get(0, result, sizeof(result));
    if (success && strcmp(result, "First") == 0) {
        printf("Verification Passed: First string matches.\n");
    } else {
        printf("Verification Failed: First string does not match.\n");
    }

    success = ringBuffer.get(1, result, sizeof(result));
    if (success && strcmp(result, "Second") == 0) {
        printf("Verification Passed: Second string matches.\n");
    } else {
        printf("Verification Failed: Second string does not match.\n");
    }

    success = ringBuffer.get(2, result, sizeof(result));
    if (success && strcmp(result, "Third") == 0) {
        printf("Verification Passed: Third string matches.\n");
    } else {
        printf("Verification Failed: Third string does not match.\n");
    }

    // Test out-of-bounds access
    success = ringBuffer.get(3, result, sizeof(result));
    printf("Get index 3 (out of bounds): (Success: %d)\n", success);
}

void testGetAndRemove() {
    printf("Running testGetAndRemove...");

    ringBuffer.clear();

    // Add strings to the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");
    ringBuffer.put("Third");

    // Retrieve and remove strings, verifying each
    char result[64];
    bool success = ringBuffer.getAndRemove(result, sizeof(result));
    if (success && strcmp(result, "First") == 0) {
        printf("Verification Passed: Removed string matches 'First'.\n");
    } else {
        printf("Verification Failed: Removed string does not match 'First'.\n");
    }

    success = ringBuffer.getAndRemove(result, sizeof(result));
    if (success && strcmp(result, "Second") == 0) {
        printf("Verification Passed: Removed string matches 'Second'.\n");
    } else {
        printf("Verification Failed: Removed string does not match 'Second'.\n");
    }

    success = ringBuffer.getAndRemove(result, sizeof(result));
    if (success && strcmp(result, "Third") == 0) {
        printf("Verification Passed: Removed string matches 'Third'.\n");
    } else {
        printf("Verification Failed: Removed string does not match 'Third'.\n");
    }

    // Test removing from an empty buffer
    success = ringBuffer.getAndRemove(result, sizeof(result));
    printf("Get and remove (empty): (Success: %d)\n", success);
}

void testConcatenate() {
    printf("Running testConcatenate...");

    ringBuffer.clear();

    // Add strings to the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");
    ringBuffer.put("Third");

    // Concatenate all strings and verify
    char result[256];
    ringBuffer.concatenate(result, sizeof(result));
    printf("Concatenated: %s\n", result);

    if (strcmp(result, "First Second Third") == 0) {
        printf("Verification Passed: Concatenated result matches.\n");
    } else {
        printf("Verification Failed: Concatenated result does not match.\n");
    }
}

void testOverwriteBehavior() {
    printf("Running testOverwriteBehavior...");

    ringBuffer.clear();

    // Fill the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");
    ringBuffer.put("Third");
    ringBuffer.put("Fourth");

    // Add another string to overwrite the oldest entry
    ringBuffer.put("Fifth");

    // Verify that the oldest string ("First") was overwritten
    char result[64];
    bool success = ringBuffer.get(0, result, sizeof(result));
    if (success && strcmp(result, "Second") == 0) {
        printf("Verification Passed: Oldest string ('First') was overwritten.\n");
    } else {
        printf("Verification Failed: Oldest string was not overwritten correctly.\n");
    }

    // Verify the rest of the buffer contents
    success = ringBuffer.get(1, result, sizeof(result));
    if (success && strcmp(result, "Third") == 0) {
        printf("Verification Passed: Second string matches 'Third'.\n");
    } else {
        printf("Verification Failed: Second string does not match 'Third'.\n");
    }

    success = ringBuffer.get(2, result, sizeof(result));
    if (success && strcmp(result, "Fourth") == 0) {
        printf("Verification Passed: Third string matches 'Fourth'.\n");
    } else {
        printf("Verification Failed: Third string does not match 'Fourth'.\n");
    }

    success = ringBuffer.get(3, result, sizeof(result));
    if (success && strcmp(result, "Fifth") == 0) {
        printf("Verification Passed: Fourth string matches 'Fifth'.\n");
    } else {
        printf("Verification Failed: Fourth string does not match 'Fifth'.\n");
    }

    // Print the buffer contents for debugging
    ringBuffer.debugPrint();
}

void testBufferState() {
    printf("Running testBufferState...");

    ringBuffer.clear();

    // Verify buffer is empty
    printf("Buffer is empty: %d\n", ringBuffer.isEmpty());
    printf("Buffer size: %zu\n", ringBuffer.size());

    // Add strings to the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");

    // Verify buffer state
    printf("Buffer is full: %d\n", ringBuffer.isFull());
    printf("Buffer size: %zu\n", ringBuffer.size());

    // Fill the buffer
    for (int i = 0; i < 6; i++) {
        ringBuffer.put("Overflow");
    }

    // Verify buffer is full
    printf("Buffer is full after overflow: %d\n", ringBuffer.isFull());
    printf("Buffer size after overflow: %zu\n", ringBuffer.size());
}

void testClear() {
    printf("Running testClear...");

    ringBuffer.clear();

    // Add strings to the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");

    // Clear the buffer
    ringBuffer.clear();

    // Verify buffer is empty
    printf("Buffer is empty after clear: %d\n", ringBuffer.isEmpty());
    printf("Buffer size after clear: %zu\n", ringBuffer.size());
}

void testDebugPrint() {
    printf("Running testDebugPrint...");

    ringBuffer.clear();

    // Add strings to the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");
    ringBuffer.put("Third");

    // Print buffer contents
    ringBuffer.debugPrint();
}

void setup() {
    //Serial.begin(115200);

    // Run all tests
    testPutAndGet();
    testGetAndRemove();
    testConcatenate();
    testOverwriteBehavior();
    testBufferState();
    testClear();
    testDebugPrint();

    printf("All tests completed.");
}

void loop() {
    // Nothing to do here
}
#endif

### **Complete Test Suite**

//* Example usage of StaticRingBuffer
//#include <Arduino.h>  
//#include "StaticRingBuffer.h"


void    setup();
void    loop();
int main()
{
    setup();
    loop();
    //exit(0);
}
#define test2
#ifdef test1
// Create a ring buffer with 8 entries, each up to 64 characters long
StaticRingBuffer<8, 64> ringBuffer;
void setup() {
    //Serial.begin(115200);

    // Add some strings to the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");
    ringBuffer.put("Third");

    // Retrieve and remove the oldest string
    char result[64];
    if (ringBuffer.getAndRemove(result, sizeof(result))) {
        printf("Retrieved and removed: %s\n", result);
    }

    // Print the remaining buffer contents
    ringBuffer.debugPrint();

    // Concatenate all remaining strings
    char concatenated[256];
    ringBuffer.concatenate(concatenated, sizeof(concatenated));
    printf("Concatenated: %s\n", concatenated);
}

void loop() {
    // Nothing to do here
}
#endif

#ifdef test2

// Create a ring buffer with 4 entries, each up to 64 characters long
StaticRingBuffer<4, 64> ringBuffer;

void testPutAndGet() {
    printf("Running testPutAndGet...\n");

    ringBuffer.clear();

    // Add strings to the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");
    ringBuffer.put("Third");

    // Retrieve strings by index and verify
    char result[64];
    bool success = ringBuffer.get(0, result, sizeof(result));
    if (success && strcmp(result, "First") == 0) {
        printf("Verification Passed: First string matches.\n");
    } else {
        printf("Verification Failed: First string does not match.\n");
    }

    success = ringBuffer.get(1, result, sizeof(result));
    if (success && strcmp(result, "Second") == 0) {
        printf("Verification Passed: Second string matches.\n");
    } else {
        printf("Verification Failed: Second string does not match.\n");
    }

    success = ringBuffer.get(2, result, sizeof(result));
    if (success && strcmp(result, "Third") == 0) {
        printf("Verification Passed: Third string matches.\n");
    } else {
        printf("Verification Failed: Third string does not match.\n");
    }

    // Test out-of-bounds access
    success = ringBuffer.get(3, result, sizeof(result));
    printf("Get index 3 (out of bounds): (Success: %d)\n", success);
}

void testGetAndRemove() {
    printf("Running testGetAndRemove...");

    ringBuffer.clear();

    // Add strings to the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");
    ringBuffer.put("Third");

    // Retrieve and remove strings, verifying each
    char result[64];
    bool success = ringBuffer.getAndRemove(result, sizeof(result));
    if (success && strcmp(result, "First") == 0) {
        printf("Verification Passed: Removed string matches 'First'.\n");
    } else {
        printf("Verification Failed: Removed string does not match 'First'.\n");
    }

    success = ringBuffer.getAndRemove(result, sizeof(result));
    if (success && strcmp(result, "Second") == 0) {
        printf("Verification Passed: Removed string matches 'Second'.\n");
    } else {
        printf("Verification Failed: Removed string does not match 'Second'.\n");
    }

    success = ringBuffer.getAndRemove(result, sizeof(result));
    if (success && strcmp(result, "Third") == 0) {
        printf("Verification Passed: Removed string matches 'Third'.\n");
    } else {
        printf("Verification Failed: Removed string does not match 'Third'.\n");
    }

    // Test removing from an empty buffer
    success = ringBuffer.getAndRemove(result, sizeof(result));
    printf("Get and remove (empty): (Success: %d)\n", success);
}

void testConcatenate() {
    printf("Running testConcatenate...");

    ringBuffer.clear();

    // Add strings to the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");
    ringBuffer.put("Third");

    // Concatenate all strings and verify
    char result[256];
    ringBuffer.concatenate(result, sizeof(result));
    printf("Concatenated: %s\n", result);

    if (strcmp(result, "First Second Third") == 0) {
        printf("Verification Passed: Concatenated result matches.\n");
    } else {
        printf("Verification Failed: Concatenated result does not match.\n");
    }
}

void testOverwriteBehavior() {
    printf("Running testOverwriteBehavior...");

    ringBuffer.clear();

    // Fill the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");
    ringBuffer.put("Third");
    ringBuffer.put("Fourth");

    // Add another string to overwrite the oldest entry
    ringBuffer.put("Fifth");

    // Verify that the oldest string ("First") was overwritten
    char result[64];
    bool success = ringBuffer.get(0, result, sizeof(result));
    if (success && strcmp(result, "Second") == 0) {
        printf("Verification Passed: Oldest string ('First') was overwritten.\n");
    } else {
        printf("Verification Failed: Oldest string was not overwritten correctly.\n");
    }

    // Verify the rest of the buffer contents
    success = ringBuffer.get(1, result, sizeof(result));
    if (success && strcmp(result, "Third") == 0) {
        printf("Verification Passed: Second string matches 'Third'.\n");
    } else {
        printf("Verification Failed: Second string does not match 'Third'.\n");
    }

    success = ringBuffer.get(2, result, sizeof(result));
    if (success && strcmp(result, "Fourth") == 0) {
        printf("Verification Passed: Third string matches 'Fourth'.\n");
    } else {
        printf("Verification Failed: Third string does not match 'Fourth'.\n");
    }

    success = ringBuffer.get(3, result, sizeof(result));
    if (success && strcmp(result, "Fifth") == 0) {
        printf("Verification Passed: Fourth string matches 'Fifth'.\n");
    } else {
        printf("Verification Failed: Fourth string does not match 'Fifth'.\n");
    }

    // Print the buffer contents for debugging
    ringBuffer.debugPrint();
}

void testBufferState() {
    printf("Running testBufferState...");

    ringBuffer.clear();

    // Verify buffer is empty
    printf("Buffer is empty: %d\n", ringBuffer.isEmpty());
    printf("Buffer size: %zu\n", ringBuffer.size());

    // Add strings to the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");

    // Verify buffer state
    printf("Buffer is full: %d\n", ringBuffer.isFull());
    printf("Buffer size: %zu\n", ringBuffer.size());

    // Fill the buffer
    for (int i = 0; i < 6; i++) {
        ringBuffer.put("Overflow");
    }

    // Verify buffer is full
    printf("Buffer is full after overflow: %d\n", ringBuffer.isFull());
    printf("Buffer size after overflow: %zu\n", ringBuffer.size());
}

void testClear() {
    printf("Running testClear...");

    ringBuffer.clear();

    // Add strings to the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");

    // Clear the buffer
    ringBuffer.clear();

    // Verify buffer is empty
    printf("Buffer is empty after clear: %d\n", ringBuffer.isEmpty());
    printf("Buffer size after clear: %zu\n", ringBuffer.size());
}

void testDebugPrint() {
    printf("Running testDebugPrint...");

    ringBuffer.clear();

    // Add strings to the buffer
    ringBuffer.put("First");
    ringBuffer.put("Second");
    ringBuffer.put("Third");

    // Print buffer contents
    ringBuffer.debugPrint();
}

void setup() {
    //Serial.begin(115200);

    // Run all tests
    testPutAndGet();
    testGetAndRemove();
    testConcatenate();
    testOverwriteBehavior();
    testBufferState();
    testClear();
    testDebugPrint();

    printf("All tests completed.");
}

void loop() {
    // Nothing to do here
}
#endif
*/