// QueueStorage.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace azure::storage;

void basic_queue_operations(utility::string_t storage_connection_string);
void list_queues(utility::string_t storage_connection_string);
void set_cors_rules(utility::string_t storage_connection_string);

int main()
{
	const utility::string_t storage_connection_string(U("UseDevelopmentStorage=true"));

	basic_queue_operations(storage_connection_string);

	list_queues(storage_connection_string);

	set_cors_rules(storage_connection_string);

	return 0;
}

void basic_queue_operations(utility::string_t storage_connection_string)
{
	// Generate unique container name
	utility::string_t block_blob_name = U("demoqueue-") + utility::uuid_to_string(utility::new_uuid());

	std::wcout << U("Creating queue") << std::endl;

	// Parse the connection string
	cloud_storage_account storage_account = cloud_storage_account::parse(storage_connection_string);

	cloud_queue_client queue_client = storage_account.create_cloud_queue_client();

	// Retrieve a reference to a queue.
	cloud_queue queue = queue_client.get_queue_reference(U("my-sample-queue"));

	try
	{
		// Create the queue if it doesn't already exist.
		queue.create_if_not_exists();
	}
	catch (const std::exception& e)
	{
		std::wcout << U("Error:") << e.what() << U(".If you are running with the default configuration, make sure the storage emulator is started.") << std::endl;
	}

	std::wcout << U("Pushing messsages to the queue") << std::endl;

	// Create a message and add it to the queue.
	cloud_queue_message message(U("Hello, World"));
	try
	{
		queue.add_message(message);

		// Add a second message to the queue
		queue.add_message(cloud_queue_message(U("Bye, World")));
	}
	catch (const std::exception& e)
	{
		std::wcout << U("Error:") << e.what() << U(".The messages could not be pushed to the queue") << std::endl;
	}

	std::wcout << U("Peeking first message from queue") << std::endl;

	try
	{
		// Peek at the next message.
		message = queue.peek_message();

		// Output the message content.
		std::wcout << U("Peeked message content: ") << message.content_as_string() << std::endl;
	}
	catch (const std::exception& e)
	{
		std::wcout << U("Error:") << e.what() << U(".The message could not be retrieved from the queue") << std::endl;
	}

	std::wcout << U("Getting message from queue") << std::endl;
	
	try
	{
		// Get the message from the queue and update the message contents.
		// The visibility timeout "0" means make it visible immediately.
		// The visibility timeout "60" means the client can get another minute to continue
		// working on the message.
		message = queue.get_message();
	}
	catch (const std::exception& e)
	{
		std::wcout << U("Error:") << e.what() << U(".The message could not be retrieved from the queue") << std::endl;
	}

	std::wcout << U("Changing message content for message ") << message.id() << std::endl;

	try 
	{
		message.set_content(U("Changed message"));
		queue.update_message(message, std::chrono::seconds(60), true);
	}
	catch (const std::exception& e)
	{
		std::wcout << U("Error:") << e.what() << U(".The message could not be updated") << std::endl;
	}

	std::wcout << U("Deleting message ") << message.id() << std::endl;
	try
	{
		// Delete the message.
		queue.delete_message(message);
	}
	catch (const std::exception& e)
	{
		std::wcout << U("Error:") << e.what() << U(".The message could not be deleted") << std::endl;
	}

	std::wcout << U("Getting messages with visibility timeout ") << std::endl;
	
	try
	{
		// Dequeue some queue messages (maximum 32 at a time) and set their visibility timeout to
		// 5 minutes (300 seconds).
		queue_request_options options;
		operation_context context;

		// Retrieve 20 messages from the queue with a visibility timeout of 300 seconds.
		std::vector<azure::storage::cloud_queue_message> messages = queue.get_messages(20, std::chrono::seconds(300), options, context);

		for (auto it = messages.cbegin(); it != messages.cend(); ++it)
		{
			// Display the contents of the message.
			std::wcout << U("Got: ") << it->content_as_string() << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::wcout << U("Error:") << e.what() << U(".The messages could not be retrieved from the queue") << std::endl;
	}

	std::wcout << U("Getting queue attributes ") << std::endl;

	try
	{
		// Fetch the queue attributes.
		queue.download_attributes();

		// Retrieve the cached approximate message count.
		int cachedMessageCount = queue.approximate_message_count();

		// Display number of messages.
		std::wcout << U("Number of messages in queue: ") << cachedMessageCount << std::endl;
	}
	catch (const std::exception& e)
	{
		std::wcout << U("Error:") << e.what() << U(".The queue attributes could not retrieved") << std::endl;
	}

	std::wcout << U("Clearing all messages from queue") << std::endl;

	try
	{
		// Clear all the messages in queue
		queue.clear();
	}
	catch (const std::exception& e)
	{
		std::wcout << U("Error:") << e.what() << U(".The queue could not be cleared") << std::endl;
	}

	std::wcout << U("Deleting queue") << std::endl;

	try
	{
		// Delete queue
		queue.delete_queue_if_exists();
	}
	catch (const std::exception& e)
	{
		std::wcout << U("Error:") << e.what() << U(".The queue could not be deleted") << std::endl;
	}
}

void list_queues(utility::string_t storage_connection_string)
{
	// Parse the connection string
	cloud_storage_account storage_account = cloud_storage_account::parse(storage_connection_string);

	cloud_queue_client queue_client = storage_account.create_cloud_queue_client();

	std::wcout << U("Listing all the available queues") << std::endl;
	queue_result_iterator end_of_results;
	for (auto it = queue_client.list_queues(); it != end_of_results; ++it)
	{
		std::wcout << U("Queue ") << it->name() << ", URI = " << it->uri().primary_uri().to_string() << std::endl;
	}
}

void set_cors_rules(utility::string_t storage_connection_string)
{
	// Parse the connection string
	cloud_storage_account storage_account = cloud_storage_account::parse(storage_connection_string);

	cloud_queue_client queue_client = storage_account.create_cloud_queue_client();

	std::wcout << U("Setting cors rules for account") << std::endl;
	try
	{
		service_properties service_properties = queue_client.download_service_properties();

		service_properties::cors_rule cors_rules = service_properties::cors_rule();
		cors_rules.allowed_origins().push_back(U("*"));
		cors_rules.allowed_methods().push_back(U("POST"));
		cors_rules.allowed_methods().push_back(U("GET"));
		cors_rules.allowed_headers().push_back(U("*"));
		cors_rules.set_max_age(std::chrono::seconds(3600));

		service_properties.cors().push_back(cors_rules);

		service_properties_includes includes;
		includes.set_cors(true);

		queue_client.upload_service_properties(service_properties, includes);
	}
	catch (const std::exception& e)
	{
		std::wcout << U("Error:") << e.what() << U(".The cors rules could not be set.") << std::endl;
	}
}
