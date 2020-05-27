#include "network_socket/tcp/socket.hpp"

namespace network_socket
{
	namespace tcp
	{
		const std::string Socket::m_STATUS_CODE_MSG[] =
		{
			"",
			"Boost error",
			"Socket closed",
			"Socket not connected",
			"Socket already open",
			"Socket already connected"
		};

		Socket::Socket(void) : m_ongoingRead(false), m_isSocketConnected(false), m_socket(m_ioContext), m_deadlineTimer(m_ioContext)
		{
			// 
			m_deadlineTimer.expires_at(boost::posix_time::pos_infin);
		}

		Socket::~Socket(void)
		{
			disconnect();
		}

		void Socket::disconnect(void)
		{
			m_socket.close();
			m_isSocketConnected = false;
		}

        Endpoint Socket::getLocalEndpoint(void) const
        {
            OperationStatus opStatus;

            return getLocalEndpoint(opStatus);
        }

        Endpoint Socket::getLocalEndpoint(OperationStatus &t_opStatus) const
        {
		    Endpoint endpoint = {"", 0};

            if (isSocketOpen())
            {
                endpoint =
                {
                    .address = m_socket.local_endpoint().address().to_string(),
                    .port = m_socket.local_endpoint().port()
                };

                t_opStatus = OperationStatus{true, StatusCode::NO_ERROR, m_STATUS_CODE_MSG[(uint16_t)StatusCode::NO_ERROR]};
            }
            else
            {
                t_opStatus = OperationStatus{false, StatusCode::SOCKET_CLOSED, m_STATUS_CODE_MSG[(uint16_t)StatusCode::SOCKET_CLOSED]};
            }

            return endpoint;
        }

        std::string Socket::getLocalEndpointAddress(void) const
        {
            OperationStatus opStatus;

            return getLocalEndpointAddress(opStatus);
        }

        std::string Socket::getLocalEndpointAddress(OperationStatus &t_opStatus) const
        {
            std::string address = "";

            if (isSocketOpen())
            {
                address = m_socket.local_endpoint().address().to_string();

                t_opStatus = OperationStatus{true, StatusCode::NO_ERROR, m_STATUS_CODE_MSG[(uint16_t)StatusCode::NO_ERROR]};
            }
            else
            {
                t_opStatus = OperationStatus{false, StatusCode::SOCKET_CLOSED, m_STATUS_CODE_MSG[(uint16_t)StatusCode::SOCKET_CLOSED]};
            }

            return address;
        }

        unsigned int Socket::getLocalEndpointPort(void) const
        {
            OperationStatus opStatus;

            return getLocalEndpointPort(opStatus);
        }

        unsigned int Socket::getLocalEndpointPort(OperationStatus &t_opStatus) const
        {
		    unsigned int port = 0;

            if (isSocketOpen())
            {
                port = m_socket.local_endpoint().port();

                t_opStatus = OperationStatus{true, StatusCode::NO_ERROR, m_STATUS_CODE_MSG[(uint16_t)StatusCode::NO_ERROR]};
            }
            else
            {
                t_opStatus = OperationStatus{false, StatusCode::SOCKET_CLOSED, m_STATUS_CODE_MSG[(uint16_t)StatusCode::SOCKET_CLOSED]};
            }

            return port;
        }

        Endpoint Socket::getRemoteEndpoint(void) const
        {
            OperationStatus opStatus;

            return getRemoteEndpoint(opStatus);
        }

        Endpoint Socket::getRemoteEndpoint(OperationStatus &t_opStatus) const
        {
            Endpoint endpoint = {"", 0};

            if (m_isSocketConnected)
            {
                endpoint =
                {
                    .address = m_socket.remote_endpoint().address().to_string(),
                    .port = m_socket.remote_endpoint().port()
                };

                t_opStatus = OperationStatus{true, StatusCode::NO_ERROR, m_STATUS_CODE_MSG[(uint16_t)StatusCode::NO_ERROR]};
            }
            else
            {
                t_opStatus = OperationStatus{false, StatusCode::SOCKET_DISCONNECTED, m_STATUS_CODE_MSG[(uint16_t)StatusCode::SOCKET_DISCONNECTED]};
            }

            return endpoint;
        }

        std::string Socket::getRemoteEndpointAddress(void) const
        {
            OperationStatus opStatus;

            return getRemoteEndpointAddress(opStatus);
        }

        std::string Socket::getRemoteEndpointAddress(OperationStatus &t_opStatus) const
        {
            std::string address = "";

            if (m_isSocketConnected)
            {
                address = m_socket.remote_endpoint().address().to_string();

                t_opStatus = OperationStatus{true, StatusCode::NO_ERROR, m_STATUS_CODE_MSG[(uint16_t)StatusCode::NO_ERROR]};
            }
            else
            {
                t_opStatus = OperationStatus{false, StatusCode::SOCKET_DISCONNECTED, m_STATUS_CODE_MSG[(uint16_t)StatusCode::SOCKET_DISCONNECTED]};
            }

            return address;
        }

        unsigned int Socket::getRemoteEndpointPort(void) const
        {
            OperationStatus opStatus;

            return getRemoteEndpointPort(opStatus);
        }

        unsigned int Socket::getRemoteEndpointPort(OperationStatus &t_opStatus) const
        {
            unsigned int port = 0;

            if (m_isSocketConnected)
            {
                port = m_socket.remote_endpoint().port();

                t_opStatus = OperationStatus{true, StatusCode::NO_ERROR, m_STATUS_CODE_MSG[(uint16_t)StatusCode::NO_ERROR]};
            }
            else
            {
                t_opStatus = OperationStatus{false, StatusCode::SOCKET_DISCONNECTED, m_STATUS_CODE_MSG[(uint16_t)StatusCode::SOCKET_DISCONNECTED]};
            }

            return port;
        }

        bool Socket::isSocketConnected(void) const
        {
			return m_isSocketConnected;
        }

        bool Socket::isSocketOpen(void) const
        {
			return m_socket.is_open();
        }

        OperationStatus Socket::openSocket(void)
        {
            try
            {
                if (!isSocketOpen())
                {
                    boost::system::error_code errorCode;

                    m_socket.open(boost::asio::ip::tcp::v4(), errorCode);

                    if (errorCode)
                    {
                        throw boost::system::system_error(errorCode);
                    }

                    return OperationStatus{true, StatusCode::NO_ERROR, m_STATUS_CODE_MSG[(uint16_t)StatusCode::NO_ERROR]};
                }
                else
                {
                    return OperationStatus{true, StatusCode::SOCKET_ALREADY_OPEN, m_STATUS_CODE_MSG[(uint16_t)StatusCode::SOCKET_ALREADY_OPEN]};
                }
            }
            catch (std::exception &e)
            {
                return OperationStatus{false, StatusCode::BOOST_ERROR, e.what()};
            }
        }

        OperationStatus Socket::read(std::string &t_message, const uint16_t &t_timeoutLimit, const size_t &t_maxSize, const size_t &t_minSize)
		{
			try
			{
				if (m_isSocketConnected)
				{
					// 
					m_deadlineTimer.expires_from_now(boost::posix_time::seconds(t_timeoutLimit));

					// 
					m_ongoingRead = true;
					
					boost::asio::streambuf buffer;
					boost::asio::streambuf::mutable_buffers_type mutableBuffer = buffer.prepare(t_maxSize);

					boost::asio::async_read(m_socket, mutableBuffer, boost::asio::transfer_at_least(t_minSize),
						boost::bind(&Socket::readHandler, this, boost::asio::placeholders::error, &buffer, boost::asio::placeholders::bytes_transferred));

					// 
					do
					{
						m_ioContext.run_one();
					} while (m_ongoingRead);

					// 
					t_message = std::string((std::istreambuf_iterator<char>(&buffer)), std::istreambuf_iterator<char>());

					// 
					return OperationStatus{true, StatusCode::NO_ERROR, m_STATUS_CODE_MSG[(uint16_t)StatusCode::NO_ERROR]};
				}
				else
				{
					return OperationStatus{false, StatusCode::SOCKET_DISCONNECTED, m_STATUS_CODE_MSG[(uint16_t)StatusCode::SOCKET_DISCONNECTED]};
				}
			}
			catch (std::exception &e)
			{
			    if (!isSocketOpen())
                {
			        m_isSocketConnected = false;

                    openSocket();
                }

				return OperationStatus{false, StatusCode::BOOST_ERROR, e.what()};
			}
		}

		OperationStatus Socket::readLine(std::string &t_message, const uint16_t &t_timeoutLimit)
		{
			try
			{
				if (m_isSocketConnected)
				{
					// 
					m_deadlineTimer.expires_from_now(boost::posix_time::seconds(t_timeoutLimit));

					// 
					boost::system::error_code errorCode = boost::asio::error::would_block;

					// 
					boost::asio::streambuf buffer;
					boost::asio::async_read_until(m_socket, buffer, '\n', boost::lambda::var(errorCode) = boost::lambda::_1);

					// 
					do
					{
						m_ioContext.run_one();
					} while (errorCode == boost::asio::error::would_block);

					if (errorCode)
					{
						throw boost::system::system_error(errorCode);
					}

					std::istream is(&buffer);
					std::getline(is, t_message);

					// 
					return OperationStatus{true, StatusCode::NO_ERROR, m_STATUS_CODE_MSG[(uint16_t)StatusCode::NO_ERROR]};
				}
				else
				{
					return OperationStatus{false, StatusCode::SOCKET_DISCONNECTED, m_STATUS_CODE_MSG[(uint16_t)StatusCode::SOCKET_DISCONNECTED]};
				}
			}
			catch (std::exception &e)
			{
                if (!isSocketOpen())
                {
                    m_isSocketConnected = false;

                    openSocket();
                }

                return OperationStatus{false, StatusCode::BOOST_ERROR, e.what()};
			}
		}

		OperationStatus Socket::write(const std::string &t_message, const uint16_t &t_timeoutLimit)
		{
			try
			{
				if (m_isSocketConnected)
				{
					// 
					m_deadlineTimer.expires_from_now(boost::posix_time::seconds(t_timeoutLimit));

					// 
					boost::system::error_code errorCode = boost::asio::error::would_block;

					// 
					boost::asio::async_write(m_socket, boost::asio::buffer(t_message), boost::lambda::var(errorCode) = boost::lambda::_1);

					// 
					do
					{
						m_ioContext.run_one();
					} while (errorCode == boost::asio::error::would_block);

					if (errorCode)
					{
						throw boost::system::system_error(errorCode);
					}

					// 
					return OperationStatus{true, StatusCode::NO_ERROR, m_STATUS_CODE_MSG[(uint16_t)StatusCode::NO_ERROR]};
				}
				else
				{
					return OperationStatus{false, StatusCode::SOCKET_DISCONNECTED, m_STATUS_CODE_MSG[(uint16_t)StatusCode::SOCKET_DISCONNECTED]};
				}
			}
			catch (std::exception &e)
			{
                if (!isSocketOpen())
                {
                    m_isSocketConnected = false;

                    openSocket();
                }

                return OperationStatus{false, StatusCode::BOOST_ERROR, e.what()};
			}
		}

		OperationStatus Socket::writeLine(const std::string &t_message, const uint16_t &t_timeoutLimit)
		{
			return write(t_message + "\n", t_timeoutLimit);
		}

		void Socket::checkDeadline(void)
		{
			// 
			if (m_deadlineTimer.expires_at() <= boost::asio::deadline_timer::traits_type::now())
			{
				// 
				boost::system::error_code ignoredErrorCode;
				m_socket.close(ignoredErrorCode);

				// 
				m_deadlineTimer.expires_at(boost::posix_time::pos_infin);
			}

			// 
			m_deadlineTimer.async_wait(boost::lambda::bind(&Socket::checkDeadline, this));
		}

		void Socket::readHandler(const boost::system::error_code &t_errorCode, boost::asio::streambuf *buffer, const std::size_t &t_bytesTransferred)
		{
			if (t_errorCode)
			{
				throw boost::system::system_error(t_errorCode ? t_errorCode : boost::asio::error::operation_aborted);
			}
			else
			{
				buffer->commit(t_bytesTransferred);
			}

			m_ongoingRead = false;
		}
	}
}
