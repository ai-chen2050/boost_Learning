#ifndef CUSTOM_TRACKING_HPP
#define CUSTOM_TRACKING_HPP

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <boost/asio.hpp>

#define BOOST_ASIO_INHERIT_TRACKED_HANDLER \
   : public ::custom_tracking::tracked_handler

#define BOOST_ASIO_ALSO_INHERIT_TRACKED_HANDLER \
   , public ::custom_tracking::tracked_handler

#define BOOST_ASIO_HANDLER_TRACKING_INIT \
   ::custom_tracking::init()

#define BOOST_ASIO_HANDLER_CREATIN(args)  \
   ::custom_tracking::creation args

#define BOOST_ASIO_HANDLER_COMPLETION(args) \
   ::custom_tracking::completion tracked_completion args

#define BOOST_ASIO_HANDLER_INVOCATION_BEGIN(args) \
  tracked_completion.invocation_begin args

# define BOOST_ASIO_HANDLER_INVOCATION_END \
  tracked_completion.invocation_end()

# define BOOST_ASIO_HANDLER_OPERATION(args) \
  ::custom_tracking::operation args

# define BOOST_ASIO_HANDLER_REACTOR_REGISTRATION(args) \
  ::custom_tracking::reactor_registration args

# define BOOST_ASIO_HANDLER_REACTOR_DEREGISTRATION(args) \
  ::custom_tracking::reactor_deregistration args

# define BOOST_ASIO_HANDLER_REACTOR_READ_EVENT 1
# define BOOST_ASIO_HANDLER_REACTOR_WRITE_EVENT 2
# define BOOST_ASIO_HANDLER_REACTOR_ERROR_EVENT 4

struct custom_tracking
{
   // Base class for objects containing tracked handlers.
   struct tracked_handler
   {
      std::uintmax_t _handler_id = 0;
      std::uintmax_t _tree_id = 0;
      const char* _object_type;
      std::uintmax_t _native_handle;
   };
   
   // Initialise the tracking system
   static void init()
   {

   }

   // Record the creation of a tracked handler.
   static void creation(boost::asio::exection_context& /*crx*/,
      tracked_handler* h,const char* object_type,void* /*object*/,
      std::uintmax_t native_handle,const char* op_name)
      {
         // Generate a unique id for the new handler.
         static std::atomic<std::uintmax_t> next_handler_id{1};
         h._handler_id = next_handler_id ++;

         // Coppy the tree identifier forward from the current handler.
         if(*current_completion())
         {
            h._tree_id = (*current_completion())->_handler._tree_id;  
         }

         // Store various attributes of the operation to use in later outpur.
         h._object_type = object_type;
         h._native_handle = native_handle;

         std::printf(
            "Starting operation %s.%s for native_handle = %" PRIuMAX 
            ",handler = %" PRIuMAX ",tree = %"PRIuMAX "\n",
            object_type, op_name, h._native_handle,h._handler_id,h._tree_id
         );
      }

      struct completion
      {
         explicit completion(const tracked_handler& h)
         :_handler(h)
         ,_next(*current_completion())
         {
            *current_completion() = this;
         }

         completion(const completion&) = delete;
         completion& operator=(const completion&) = delete;

         ~completion()
         {
            *current_completion() = _next;
         }

         // Records that handler is to be invoked with the specified arguments 
         template <typename... Args>
         void invocation_begin(Args&&... /*args*/)
         {
            std::printf("Entering handler %" PRIuMAX " in tree %" PRIuMAX "\n",
                  _handler._handler_id,_handler._tree_id);
         }

         // Record that handler invocation has ended 
         void invocation_end()
         {
            std::printf("Leaving handler %" PRIuMAX " in tree %" PRIuMAX "\n",
                  _handler._handler_id,_handler._tree_id);
         }

         tracked_handler _handler;

         // Completions may nest. Here wo stash a pointer to the outer completion.
         completion* _next;
      };
      
   static completion** current_completion()
   {
      static BOOST_ASIO_THREAD_KEYWORD completion* current = nullptr;
      return &current;
   }

   // Recoed an operation that is not directly associated with a handler.
   static void operation(boost::asio::exection_context& /*ctx*/,
      const char* /*object_type*/,void* /*object*/,
      std::uintmax_t /*native_handle*/,const char* /*op_name*/)
   {

   }

   // Record that a descriptor has been registered with the reactor.
   static void reactor_registration(boost::asio::exection_context& context,
      uintmax_t native_handle,uintmax_t registration)
   {
      std::printf("Adding to reactor native_handle = %" PRIuMAX
         ",registration = %" PRIuMAX "\n",native_handle,registration);
   }

    // Record that a descriptor has been deregistered from the reactor.
  static void reactor_deregistration(boost::asio::execution_context& context,
      uintmax_t native_handle, uintmax_t registration)
  {
    std::printf("Removing from reactor native_handle = %" PRIuMAX
        ", registration = %" PRIuMAX "\n", native_handle, registration);
  }

   // Record reactor-based readingess
   static void reactor_events(boost::asio::execution_context& context,
      uintmax_t registration, unsigned events)
   {
      std::printf(
         "Reactor readiness for registration = %" PRIuMAX ", events = %s%s%s\n",
         registration,
         (events & BOOST_ASIO_HANDLER_REACTOR_READ_EVENT) ? " read" : "",
         (events & BOOST_ASIO_HANDLER_REACTOR_WRITE_EVENT) ? " write": "",
         (events & BOOST_ASIO_HANDLER_REACTOR_ERROR_EVENT) ? " error": ""
      );
   }

   // Record a reactor-based operation that is associated with a handler.
   static void reactor_operation(const tracked_handler& h,
      const char* op_name,const boost::system::error_code& ec)
   {
      std::printf(
         "Performed operation %s.%s for native_handle = %" PRIuMAX
         ", ec = %s:%d\n", h._object_type,op_name,h._native_handle,
         ec.category().name(),ec.value());
   }

   // Record a reactor-based operation that is associated with a handler.
  static void reactor_operation(const tracked_handler& h,
      const char* op_name, const boost::system::error_code& ec,
      std::size_t bytes_transferred)
  {
    std::printf(
        "Performed operation %s.%s for native_handle = %" PRIuMAX
        ", ec = %s:%d, n = %" PRIuMAX "\n", h.object_type_, op_name,
        h.native_handle_, ec.category().name(), ec.value(),
        static_cast<uintmax_t>(bytes_transferred));
  }

};

#endif 