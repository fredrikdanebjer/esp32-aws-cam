# AWS Interface

The FSY-Eye connects to AWS for two reasons, to upload status messages and images, and to expose an interface from which it can be configured.

Uploaded images are not a stream and sent with a relatively low frequency, with the purpose of long-term storage.

## Configuration commands

The FSU-Eye can process commands that targets it services according to sc_service_list_t, as defined in include/system_controller.h. An action needs to be command on that service, as defined in relevant service file.
A command message has the following form:
{
  "id":<id>,
  "service_id":<service>,
  "command_id":<command>
}

More info to be added.
