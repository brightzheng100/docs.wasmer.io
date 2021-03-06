#include <stdio.h>
#include "wasmer.h"
#include <assert.h>
#include <string.h>

// Function to print the most recent error string from Wasmer if we have them
void print_wasmer_error()
{
  int error_len = wasmer_last_error_length();
  char *error_str = malloc(error_len);
  wasmer_last_error_message(error_str, error_len);
  printf("Error: `%s`\n", error_str);
}

// Function to create a Wasmer Instance
wasmer_instance_t *create_wasmer_instance() {

  // Create module name for our imports

  // Create a UTF-8 string as bytes for our module name. 
  // And, place the string into the wasmer_byte_array type so it can be used by our guest Wasm instance.
  const char *module_name = "env";
  wasmer_byte_array module_name_bytes = { .bytes = (const uint8_t *) module_name,
    .bytes_len = strlen(module_name) };

  // Define an array containing our imports
  wasmer_import_t imports[] = {};

  // Read the Wasm file bytes
  FILE *file = fopen("../../../shared/c/handling-errors.wasm", "r");
  assert(file != NULL);
  fseek(file, 0, SEEK_END);
  long len = ftell(file);
  uint8_t *bytes = malloc(len);
  fseek(file, 0, SEEK_SET);
  fread(bytes, 1, len, file);
  fclose(file);

  // Instantiate a WebAssembly Instance from Wasm bytes and imports
  wasmer_instance_t *instance = NULL;
  wasmer_result_t compile_result = wasmer_instantiate(
      &instance, // Our reference to our Wasm instance 
      bytes, // The bytes of the WebAssembly modules
      len, // The length of the bytes of the WebAssembly module
      imports, // The Imports array the will be used as our importObject
      0 // The number of imports in the imports array
      );

  // Ensure the compilation was successful.
  if (compile_result != WASMER_OK)
  {
    print_wasmer_error();
  }

  // Assert the Wasm instantion completed
  assert(compile_result == WASMER_OK);

  // Return the Wasmer Instance
  return instance;
}

int main() {

  // Initialize our Wasmer Instance
  wasmer_instance_t *instance = create_wasmer_instance();

  // Let's call the our throw_wasm_error Function in the guest Wasm module

  // Define our results. Results are created with { 0 } to avoid null issues,
  // And will be filled with the proper result after calling the guest Wasm function.
  wasmer_value_t result_one = { 0 };
  wasmer_value_t results[] = {result_one};

  // Define our parameters (none) we are passing into the guest Wasm function call.
  wasmer_value_t params[] = {0};


  // Call the Wasm function
  wasmer_result_t call_result = wasmer_instance_call(
      instance, // Our Wasm Instance
      "throw_wasm_error", // the name of the exported function we want to call on the guest Wasm module
      params, // Our array of parameters
      0, // The number of parameters
      results, // Our array of results
      1 // The number of results
      );

  // Assert that the guest Wasm function call Error'd
  // Checking both WASMER_OK and WASMER_ERROR just for
  // demonstrating the two statuses exposed by the header file.
  assert(call_result != WASMER_OK);
  assert(call_result == WASMER_ERROR);

  // Print out what happened (So it is not confusing to see an error).
  printf("The Guest Wasm Function \"throw_wasm_error\" threw and error like we expected!\n");
  printf("Printing the error ...\n\n");

  // Print out the error
  print_wasmer_error();

  // Confirming everything ran as expected!
  printf("\nTest ran sucessfully, ending execution ...\n");

  // Destroy the instances we created for our wasmer
  wasmer_instance_destroy(instance);
  return 0;
}
