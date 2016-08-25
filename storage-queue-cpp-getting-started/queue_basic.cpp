//----------------------------------------------------------------------------------
// Microsoft Developer & Platform Evangelism
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
// EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES 
// OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
//----------------------------------------------------------------------------------
// The example companies, organizations, products, domain names,
// e-mail addresses, logos, people, places, and events depicted
// herein are fictitious.  No association with any real company,
// organization, product, domain name, email address, logo, person,
// places, or events is intended or should be inferred.
//----------------------------------------------------------------------------------

#include "stdafx.h"
#include "queue_basic.h"
#include "string_util.h"

using namespace azure::storage;

///
/// This shows basic operations for queues such as queuing, dequeing or peeking messages.
///
void queue_basic::queue_operations(cloud_queue_client queue_client)
{
    try
    {
        ucout << U("Creating queue") << std::endl;
        cloud_queue queue = queue_client.get_queue_reference(U("my-sample-queue"));

        queue.create_if_not_exists();

        ucout << U("Pushing messsages to the queue") << std::endl;
        
        // Create a message and add it to the queue.
        cloud_queue_message first_message(U("First Message"));
        queue.add_message(first_message);

        // Add more messages to the queue
        for (int i = 0; i < 5; i++)
        {
            // Create a message and add it to the queue.
            cloud_queue_message message(U("other message ") + string_util::random_string());
            queue.add_message(message);
        }

        // Storage queues are not strictly ordered so while you'll almost always get the first message 'First Message', 
        // you may occasionally get one of the other messages first.
        ucout << U("Peeking first message from queue") << std::endl;
        cloud_queue_message peek_message = queue.peek_message();

        ucout << U("Peeked message content: ") << peek_message.content_as_string() << std::endl;

        ucout << U("Getting message from queue") << std::endl;
        cloud_queue_message update_message = queue.get_message();

        ucout << U("Changing message content for message ") << update_message.id() << std::endl;
        update_message.set_content(U("Changed message"));
        queue.update_message(update_message, std::chrono::seconds(60), true);

        ucout << U("Deleting message ") << update_message.id() << std::endl;
        queue.delete_message(update_message);

        ucout << U("Getting messages with visibility timeout ") << std::endl;
        // Dequeue some queue messages (maximum 32 at a time) and set their visibility timeout to
        // 5 minutes (300 seconds).
        queue_request_options options;
        operation_context context;

        // Retrieve 20 messages from the queue with a visibility timeout of 300 seconds.
        std::vector<azure::storage::cloud_queue_message> messages = queue.get_messages(20, std::chrono::seconds(300), options, context);
        for (auto it = messages.cbegin(); it != messages.cend(); ++it)
        {
            // Display the contents of the message.
            ucout << U("Got: ") << it->content_as_string() << std::endl;
        }

        ucout << U('Getting queue attributes') << std::endl;
        queue.download_attributes();

        // Retrieve the cached approximate message count.
        int cachedMessageCount = queue.approximate_message_count();

        ucout << U("Number of messages in queue: ") << cachedMessageCount << std::endl;

        ucout << U("Clearing all messages from queue") << std::endl;
        queue.clear();

        ucout << U("Deleting queue") << std::endl;
        queue.delete_queue_if_exists();
    }
    catch (const azure::storage::storage_exception& e)
    {
        ucout << U("Error: ") << e.what() << " .Extended error:" << e.result().extended_error().message() << std::endl << std::endl;
    }
    catch (const std::exception& e)
    {
        ucout << U("Error:") << e.what() << std::endl << std::endl;
    }
}
