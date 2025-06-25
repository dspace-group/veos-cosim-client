// Copyright dSPACE GmbH. All rights reserved.

#pragma once

// NOLINTBEGIN

#include <stdint.h>

#ifdef __cplusplus
#include <cstddef>
#include <string>
#endif

#if (defined __GNUC__) && (!defined _DOXYGEN)
#ifdef DSVEOSCOSIM_EXPORT
#define DSVEOSCOSIM_API __attribute__((__visibility__("default")))
#else
#define DSVEOSCOSIM_API
#endif
#elif (defined _MSC_VER)
#if defined(DSVEOSCOSIM_EXPORT)
#define DSVEOSCOSIM_API __declspec(dllexport)
#elif defined(DSVEOSCOSIM_IMPORT)
#define DSVEOSCOSIM_API __declspec(dllimport)
#else
#define DSVEOSCOSIM_API
#endif
#else
#define DSVEOSCOSIM_API
#endif

#ifdef __cplusplus
#define DSVEOSCOSIM_EXTERN extern "C"
#else
#define DSVEOSCOSIM_EXTERN extern
#endif

#define DSVEOSCOSIM_DECL DSVEOSCOSIM_EXTERN DSVEOSCOSIM_API

/**
 * \brief Defines the conversion factor between DsVeosCoSim_SimulationTime and seconds in data type double.
 */
#define DSVEOSCOSIM_SIMULATION_TIME_RESOLUTION_PER_SECOND 1e9

/**
 * \brief Converts the given simulation time to a double in seconds.
 * \param simulationTime    The simulation time to convert.
 */
#define DSVEOSCOSIM_SIMULATION_TIME_TO_SECONDS(simulationTime) \
    ((double)(simulationTime) / DSVEOSCOSIM_SIMULATION_TIME_RESOLUTION_PER_SECOND)

/**
 * \brief Defines the maximum length of a CAN message payload.
 */
enum {
    DSVEOSCOSIM_CAN_MESSAGE_MAX_LENGTH = 64
};

/**
 * \brief Defines the maximum length of an ethernet message payload.
 */
enum {
    DSVEOSCOSIM_ETH_MESSAGE_MAX_LENGTH = 9018
};

/**
 * \brief Defines the maximum length of a LIN message payload.
 */
enum {
    DSVEOSCOSIM_LIN_MESSAGE_MAX_LENGTH = 8
};

/**
 * \brief Defines the maximum length of an ethernet address (MAC address).
 */
enum {
    DSVEOSCOSIM_ETH_ADDRESS_LENGTH = 6
};

/**
 * \brief Represents a handle to be used for communicating with a dSPACE VEOS CoSim server.
 */
typedef void* DsVeosCoSim_Handle;

/**
 * \brief Represents an IO signal id.
 */
typedef uint32_t DsVeosCoSim_IoSignalId;

/**
 * \brief Represents a bus controller id.
 */
typedef uint32_t DsVeosCoSim_BusControllerId;

/**
 * \brief Represents the simulation time in nanoseconds.
 */
typedef int64_t DsVeosCoSim_SimulationTime;

/**
 * \brief Represents a result of a function.
 */
typedef enum DsVeosCoSim_Result {
    /**
     * \brief Will be returned if the API function was successful.
     */
    DsVeosCoSim_Result_Ok,

    /**
     * \brief Will be returned if the API function finished with a generic error.
     */
    DsVeosCoSim_Result_Error,

    /**
     * \brief Will be returned if the reception API function found an empty buffer.
     */
    DsVeosCoSim_Result_Empty,

    /**
     * \brief Will be returned if the transmit API function found a full buffer.
     */
    DsVeosCoSim_Result_Full,

    /**
     * \brief Will be returned if the argument to an API function was invalid.
     */
    DsVeosCoSim_Result_InvalidArgument,

    /**
     * \brief Will be returned if the API function detected a connection lost to the dSPACE VEOS CoSim server.
     */
    DsVeosCoSim_Result_Disconnected,

    DsVeosCoSim_Result_INT_MAX_SENTINEL_DO_NOT_USE_ = INT32_MAX
} DsVeosCoSim_Result;

/**
 * \brief Represents a command for the non-blocking API.
 */
typedef enum DsVeosCoSim_Command {
    /**
     * \brief No command.
     */
    DsVeosCoSim_Command_None,

    /**
     * \brief Simulation step command.
     */
    DsVeosCoSim_Command_Step,

    /**
     * \brief Simulation start command.
     */
    DsVeosCoSim_Command_Start,

    /**
     * \brief Simulation stop command.
     */
    DsVeosCoSim_Command_Stop,

    /**
     * \brief Simulation terminate command.
     */
    DsVeosCoSim_Command_Terminate,

    /**
     * \brief Simulation pause command.
     */
    DsVeosCoSim_Command_Pause,

    /**
     * \brief Simulation continue command.
     */
    DsVeosCoSim_Command_Continue,

    DsVeosCoSim_Command_INT_MAX_SENTINEL_DO_NOT_USE_ = INT32_MAX
} DsVeosCoSim_Command;

/**
 * \brief Represents the severity of a log message.
 */
typedef enum DsVeosCoSim_Severity {
    /**
     * \brief Error message.
     */
    DsVeosCoSim_Severity_Error,

    /**
     * \brief Warning message.
     */
    DsVeosCoSim_Severity_Warning,

    /**
     * \brief Information message.
     */
    DsVeosCoSim_Severity_Info,

    /**
     * \brief Trace message.
     */
    DsVeosCoSim_Severity_Trace,

    DsVeosCoSim_Severity_INT_MAX_SENTINEL_DO_NOT_USE_ = INT32_MAX
} DsVeosCoSim_Severity;

/**
 * \brief Represents the reason of a simulation termination.
 */
typedef enum DsVeosCoSim_TerminateReason {
    /**
     * \brief Simulation finished successfully.
     */
    DsVeosCoSim_TerminateReason_Finished,

    /**
     * \brief Simulation terminated with an error.
     */
    DsVeosCoSim_TerminateReason_Error,

    DsVeosCoSim_TerminateReason_INT_MAX_SENTINEL_DO_NOT_USE_ = INT32_MAX
} DsVeosCoSim_TerminateReason;

/**
 * \brief Represents the connection state.
 */
typedef enum DsVeosCoSim_ConnectionState {
    /**
     * \brief Disconnected.
     */
    DsVeosCoSim_ConnectionState_Disconnected,

    /**
     * \brief Connected.
     */
    DsVeosCoSim_ConnectionState_Connected,

    DsVeosCoSim_ConnectionState_INT_MAX_SENTINEL_DO_NOT_USE_ = INT32_MAX
} DsVeosCoSim_ConnectionState;

/**
 * \brief Represents the data type of IO signal.
 */
typedef enum DsVeosCoSim_DataType {
    /**
     * \brief Boolean. Underlying data type is uint8. 0 is equal to false, != 0 is equal to true.
     */
    DsVeosCoSim_DataType_Bool = 1,

    /**
     * \brief Signed integer with 8 bits.
     */
    DsVeosCoSim_DataType_Int8,

    /**
     * \brief Signed integer with 16 bits.
     */
    DsVeosCoSim_DataType_Int16,

    /**
     * \brief Signed integer with 32 bits.
     */
    DsVeosCoSim_DataType_Int32,

    /**
     * \brief Signed integer with 64 bits.
     */
    DsVeosCoSim_DataType_Int64,

    /**
     * \brief Unsigned integer with 8 bits.
     */
    DsVeosCoSim_DataType_UInt8,

    /**
     * \brief Unsigned integer with 16 bits.
     */
    DsVeosCoSim_DataType_UInt16,

    /**
     * \brief Unsigned integer with 32 bits.
     */
    DsVeosCoSim_DataType_UInt32,

    /**
     * \brief Unsigned integer with 64 bits.
     */
    DsVeosCoSim_DataType_UInt64,

    /**
     * \brief Floating point with 32 bits.
     */
    DsVeosCoSim_DataType_Float32,

    /**
     * \brief Floating point with 64 bits.
     */
    DsVeosCoSim_DataType_Float64,

    DsVeosCoSim_DataType_INT_MAX_SENTINEL_DO_NOT_USE_ = INT32_MAX
} DsVeosCoSim_DataType;

/**
 * \brief Represents the size kind of IO signal.
 */
typedef enum DsVeosCoSim_SizeKind {
    /**
     * \brief The IO signal size is fixed.
     */
    DsVeosCoSim_SizeKind_Fixed = 1,

    /**
     * \brief The IO signal size is variable.
     */
    DsVeosCoSim_SizeKind_Variable,

    DsVeosCoSim_SizeKind_INT_MAX_SENTINEL_DO_NOT_USE_ = INT32_MAX
} DsVeosCoSim_SizeKind;

/**
 * \brief Represents an IO signal.
 */
typedef struct DsVeosCoSim_IoSignal {
    /**
     * \brief Unique id of the IO signal.
     */
    DsVeosCoSim_IoSignalId id;

    /**
     * \brief Exact length of a fixed sized IO signal.
     *        Maximum length of a variable sized IO signal.
     */
    uint32_t length;

    /**
     * \brief Data type of the IO signal.
     */
    DsVeosCoSim_DataType dataType;

    /**
     * \brief Size kind of the IO signal.
     */
    DsVeosCoSim_SizeKind sizeKind;

    /**
     * \brief Name of the IO signal.
     */
    const char* name;
} DsVeosCoSim_IoSignal;

/**
 * \brief Represents a CAN controller.
 */
typedef struct DsVeosCoSim_CanController {
    /**
     * \brief Unique id of the CAN controller.
     */
    DsVeosCoSim_BusControllerId id;

    /**
     * \brief Maximum queue size of the CAN controller.
     */
    uint32_t queueSize;

    /**
     * \brief Bits per second of the CAN controller.
     */
    uint64_t bitsPerSecond;

    /**
     * \brief Bits per second for CAN FD of the CAN controller.
     */
    uint64_t flexibleDataRateBitsPerSecond;

    /**
     * \brief Name of the CAN controller.
     */
    const char* name;

    /**
     * \brief Name of the CAN channel.
     */
    const char* channelName;

    /**
     * \brief Name of the CAN cluster.
     */
    const char* clusterName;
} DsVeosCoSim_CanController;

/**
 * \brief Underlying data type of the flags of a CAN message.
 */
typedef uint32_t DsVeosCoSim_CanMessageFlags;

/**
 * \brief Represents the flags of a CAN message.
 */
enum {
    /**
     * \brief CAN message will be transmitted to sender as well.
     *        For received and transmitted messages.
     */
    DsVeosCoSim_CanMessageFlags_Loopback = 1,

    /**
     * \brief CAN message could not be transmitted due to an error at the dSPACE VEOS CoSim server.
     *        For received messages only.
     */
    DsVeosCoSim_CanMessageFlags_Error = 2,

    /**
     * \brief CAN message was dropped due to a full buffer at the dSPACE VEOS CoSim server.
     *        For received messages only.
     */
    DsVeosCoSim_CanMessageFlags_Drop = 4,

    /**
     * \brief CAN message id uses the extended range.
     *        For received and transmitted messages.
     */
    DsVeosCoSim_CanMessageFlags_ExtendedId = 8,

    /**
     * \brief CAN message has a bit rate switch.
     *        For received and transmitted messages.
     */
    DsVeosCoSim_CanMessageFlags_BitRateSwitch = 16,

    /**
     * \brief CAN message is a CANFD message.
     *        For received and transmitted messages.
     */
    DsVeosCoSim_CanMessageFlags_FlexibleDataRateFormat = 32
};

/**
 * \brief Represents a CAN message.
 */
typedef struct DsVeosCoSim_CanMessage {
    /**
     * \brief The simulation time when the CAN message was received.
     *        For received messages only.
     */
    DsVeosCoSim_SimulationTime timestamp;

    /**
     * \brief Unique id of the CAN controller.
     */
    DsVeosCoSim_BusControllerId controllerId;

    /**
     * \brief CAN message ID.
     */
    uint32_t id;

    /**
     * \brief CAN message flags.
     */
    DsVeosCoSim_CanMessageFlags flags;

    /**
     * \brief Payload length.
     */
    uint32_t length;

    /**
     * \brief Payload.
     */
    const uint8_t* data;
} DsVeosCoSim_CanMessage;

/**
 * \brief Represents an ethernet controller.
 */
typedef struct DsVeosCoSim_EthController {
    /**
     * \brief Unique id of the ethernet controller.
     */
    DsVeosCoSim_BusControllerId id;

    /**
     * \brief Maximum queue size of the ethernet controller.
     */
    uint32_t queueSize;

    /**
     * \brief Bits per second of the ethernet controller.
     */
    uint64_t bitsPerSecond;

    /**
     * \brief MAC address of the ethernet controller.
     */
    uint8_t macAddress[DSVEOSCOSIM_ETH_ADDRESS_LENGTH];

    /**
     * \brief Name of the ethernet controller.
     */
    const char* name;

    /**
     * \brief Name of the ethernet channel.
     */
    const char* channelName;

    /**
     * \brief Name of the ethernet cluster.
     */
    const char* clusterName;
} DsVeosCoSim_EthController;

/**
 * \brief Underlying data type of the flags of an ethernet message.
 */
typedef uint32_t DsVeosCoSim_EthMessageFlags;

/**
 * \brief Represents the flags of an ethernet message.
 */
enum {
    /**
     * \brief Ethernet message will be transmitted to sender as well.
     *        For received and transmitted messages.
     */
    DsVeosCoSim_EthMessageFlags_Loopback = 1,

    /**
     * \brief Ethernet message could not be transmitted due to an error at the dSPACE VEOS CoSim server.
     *        For received messages only.
     */
    DsVeosCoSim_EthMessageFlags_Error = 2,

    /**
     * \brief Ethernet message was dropped due to a full buffer at the dSPACE VEOS CoSim server.
     *        For received messages only.
     */
    DsVeosCoSim_EthMessageFlags_Drop = 4
};

/**
 * \brief Represents an ethernet message.
 */
typedef struct DsVeosCoSim_EthMessage {
    /**
     * \brief The simulation time when the ethernet message was received.
     *        For received messages only.
     */
    DsVeosCoSim_SimulationTime timestamp;

    /**
     * \brief Unique id of the ethernet controller.
     */
    DsVeosCoSim_BusControllerId controllerId;

    /**
     * \brief Reserved for future use.
     */
    uint32_t reserved;

    /**
     * \brief Ethernet message flags.
     */
    DsVeosCoSim_EthMessageFlags flags;

    /**
     * \brief Payload length.
     */
    uint32_t length;

    /**
     * \brief Payload.
     */
    const uint8_t* data;
} DsVeosCoSim_EthMessage;

/**
 * \brief Represents the type of the LIN controller.
 */
typedef enum DsVeosCoSim_LinControllerType {
    /**
     * \brief LIN controller is a responder.
     */
    DsVeosCoSim_LinControllerType_Responder = 1,

    /**
     * \brief LIN controller is a commander.
     */
    DsVeosCoSim_LinControllerType_Commander,

    DsVeosCoSim_LinControllerType_INT_MAX_SENTINEL_DO_NOT_USE_ = INT32_MAX
} DsVeosCoSim_LinControllerType;

/**
 * \brief Represents an LIN controller.
 */
typedef struct DsVeosCoSim_LinController {
    /**
     * \brief Unique id of the LIN controller.
     */
    DsVeosCoSim_BusControllerId id;

    /**
     * \brief Maximum queue size of the LIN controller.
     */
    uint32_t queueSize;

    /**
     * \brief Bits per second of the LIN controller.
     */
    uint64_t bitsPerSecond;

    /**
     * \brief Specified the type of the LIN controller.
     */
    DsVeosCoSim_LinControllerType type;

    /**
     * \brief Name of the LIN controller.
     */
    const char* name;

    /**
     * \brief Name of the LIN channel.
     */
    const char* channelName;

    /**
     * \brief Name of the LIN cluster.
     */
    const char* clusterName;
} DsVeosCoSim_LinController;

/**
 * \brief Underlying data type of the flags of a LIN message.
 */
typedef uint32_t DsVeosCoSim_LinMessageFlags;

/**
 * \brief Represents the flags of a LIN message.
 */
enum {
    /**
     * \brief LIN message will be transmitted to sender as well.
     *        For received and transmitted messages.
     */
    DsVeosCoSim_LinMessageFlags_Loopback = 1,

    /**
     * \brief LIN message could not be transmitted due to an error at the dSPACE VEOS CoSim server.
     *        For received messages only.
     */
    DsVeosCoSim_LinMessageFlags_Error = 2,

    /**
     * \brief LIN message was dropped due to a full buffer at the dSPACE VEOS CoSim server.
     *        For received messages only.
     */
    DsVeosCoSim_LinMessageFlags_Drop = 4,

    /**
     * \brief LIN message is a header.
     *        For received and transmitted messages.
     */
    DsVeosCoSim_LinMessageFlags_Header = 8,

    /**
     * \brief LIN message is a response.
     *        For received and transmitted messages.
     */
    DsVeosCoSim_LinMessageFlags_Response = 16,

    /**
     * \brief LIN message is a wake event.
     *        For received and transmitted messages.
     */
    DsVeosCoSim_LinMessageFlags_WakeEvent = 32,

    /**
     * \brief LIN message is a sleep event.
     *        For received and transmitted messages.
     */
    DsVeosCoSim_LinMessageFlags_SleepEvent = 64,

    /**
     * \brief LIN message uses the enhanced checksum.
     *        For received and transmitted messages.
     */
    DsVeosCoSim_LinMessageFlags_EnhancedChecksum = 128,

    /**
     * \brief LIN message will only be used for the next header.
     *        For transmitted messages only.
     */
    DsVeosCoSim_LinMessageFlags_TransferOnce = 256,

    /**
     * \brief LIN header could not be transmitted. Another LIN header was sent at the same time.
     *        For received messages only.
     */
    DsVeosCoSim_LinMessageFlags_ParityFailure = 512,

    /**
     * \brief LIN response could not be transmitted. Another LIN response was sent at the same time.
     *        For received messages only.
     */
    DsVeosCoSim_LinMessageFlags_Collision = 1024,

    /**
     * \brief No response received for the last header.
     *        For received messages only.
     */
    DsVeosCoSim_LinMessageFlags_NoResponse = 2048
};

/**
 * \brief Represents a LIN message.
 */
typedef struct DsVeosCoSim_LinMessage {
    /**
     * \brief The simulation time when the LIN message was received.
     *        For received messages only.
     */
    DsVeosCoSim_SimulationTime timestamp;

    /**
     * \brief Unique id of the bus controller.
     */
    DsVeosCoSim_BusControllerId controllerId;

    /**
     * \brief LIN message ID.
     */
    uint32_t id;

    /**
     * \brief LIN message flags.
     */
    DsVeosCoSim_LinMessageFlags flags;

    /**
     * \brief Payload length.
     */
    uint32_t length;

    /**
     * \brief Payload.
     */
    const uint8_t* data;
} DsVeosCoSim_LinMessage;

/**
 * \brief Represents the log callback function pointer.
 * \param severity      The severity of the message.
 * \param logMessage    The log message.
 */
typedef void (*DsVeosCoSim_LogCallback)(DsVeosCoSim_Severity severity, const char* logMessage);

/**
 * \brief Represents a simulation state changed or step callback function pointer.
 * \param simulationTime    The current simulation time.
 * \param userData          The user data passed via DsVeosCoSim_SetCallbacks.
 */
typedef void (*DsVeosCoSim_SimulationCallback)(DsVeosCoSim_SimulationTime simulationTime, void* userData);

/**
 * \brief Represents a simulation terminated callback function pointer.
 * \param simulationTime    The current simulation time.
 * \param reason            The termination reason.
 * \param userData          The user data passed via DsVeosCoSim_SetCallbacks.
 */
typedef void (*DsVeosCoSim_SimulationTerminatedCallback)(DsVeosCoSim_SimulationTime simulationTime,
                                                         DsVeosCoSim_TerminateReason reason,
                                                         void* userData);

/**
 * \brief Represents an incoming signal changed callback function pointer.
 * \param simulationTime    The current simulation time.
 * \param incomingSignal    The IO signal that changed.
 * \param length            The length of the changed data.
 * \param value             The changed data.
 * \param userData          The user data passed via DsVeosCoSim_SetCallbacks.
 */
typedef void (*DsVeosCoSim_IncomingSignalChangedCallback)(DsVeosCoSim_SimulationTime simulationTime,
                                                          const DsVeosCoSim_IoSignal* incomingSignal,
                                                          uint32_t length,
                                                          const void* value,
                                                          void* userData);

/**
 * \brief Represents a CAN message received callback function pointer.
 * \param simulationTime    The current simulation time.
 * \param canController     The CAN controller transmitting the message.
 * \param message           The received message.
 * \param userData          The user data passed via DsVeosCoSim_SetCallbacks.
 */
typedef void (*DsVeosCoSim_CanMessageReceivedCallback)(DsVeosCoSim_SimulationTime simulationTime,
                                                       const DsVeosCoSim_CanController* canController,
                                                       const DsVeosCoSim_CanMessage* message,
                                                       void* userData);

/**
 * \brief Represents an ethernet message received callback function pointer.
 * \param simulationTime    The current simulation time.
 * \param ethController     The ethernet controller transmitting the message.
 * \param message           The received message.
 * \param userData          The user data passed via DsVeosCoSim_SetCallbacks.
 */
typedef void (*DsVeosCoSim_EthMessageReceivedCallback)(DsVeosCoSim_SimulationTime simulationTime,
                                                       const DsVeosCoSim_EthController* ethController,
                                                       const DsVeosCoSim_EthMessage* message,
                                                       void* userData);

/**
 * \brief Represents a LIN message received callback function pointer.
 * \param simulationTime    The current simulation time.
 * \param linController     The LIN controller transmitting the message.
 * \param message           The received message.
 * \param userData          The user data passed via DsVeosCoSim_SetCallbacks.
 */
typedef void (*DsVeosCoSim_LinMessageReceivedCallback)(DsVeosCoSim_SimulationTime simulationTime,
                                                       const DsVeosCoSim_LinController* linController,
                                                       const DsVeosCoSim_LinMessage* message,
                                                       void* userData);

/**
 * \brief Represents the callbacks that will be fired during the co-simulation.
 */
typedef struct DsVeosCoSim_Callbacks {
    /**
     * \brief Will be called when the simulation started in dSPACE VEOS.
     */
    DsVeosCoSim_SimulationCallback simulationStartedCallback;

    /**
     * \brief Will be called when the simulation stopped in dSPACE VEOS.
     */
    DsVeosCoSim_SimulationCallback simulationStoppedCallback;

    /**
     * \brief Will be called when the simulation terminated in dSPACE VEOS.
     */
    DsVeosCoSim_SimulationTerminatedCallback simulationTerminatedCallback;

    /**
     * \brief Will be called when the simulation paused in dSPACE VEOS.
     */
    DsVeosCoSim_SimulationCallback simulationPausedCallback;

    /**
     * \brief Will be called when the simulation continued in dSPACE VEOS.
     */
    DsVeosCoSim_SimulationCallback simulationContinuedCallback;

    /**
     * \brief Will be called at the beginning of a simulation step performed by dSPACE VEOS.
     */
    DsVeosCoSim_SimulationCallback simulationBeginStepCallback;

    /**
     * \brief Will be called at the end of a simulation step performed by dSPACE VEOS.
     */
    DsVeosCoSim_SimulationCallback simulationEndStepCallback;

    /**
     * \brief Will be called when an incoming signal value changed.
     */
    DsVeosCoSim_IncomingSignalChangedCallback incomingSignalChangedCallback;

    /**
     * \brief Will be called when a CAN message was received from dSPACE VEOS.
     *        If this callback is registered, then DsVeosCoSim_ReceiveCanMessage will always return
     *        DsVeosCoSim_Result_Empty.
     */
    DsVeosCoSim_CanMessageReceivedCallback canMessageReceivedCallback;

    /**
     * \brief Will be called when an ethernet message was received from dSPACE VEOS.
     *        If this callback is registered, then DsVeosCoSim_ReceiveEthMessage will always return
     *        DsVeosCoSim_Result_Empty.
     */
    DsVeosCoSim_EthMessageReceivedCallback ethMessageReceivedCallback;

    /**
     * \brief Will be called when a LIN message was received from dSPACE VEOS.
     *        If this callback is registered, then DsVeosCoSim_ReceiveLinMessage will always return
     *        DsVeosCoSim_Result_Empty.
     */
    DsVeosCoSim_LinMessageReceivedCallback linMessageReceivedCallback;

    /**
     * \brief An arbitrary object that will be passed to every callback.
     */
    void* userData;
} DsVeosCoSim_Callbacks;

/**
 * \brief Represents the data that will be used for establishing the connection.
 */
typedef struct DsVeosCoSim_ConnectConfig {
    /**
     * \brief The IP address of the dSPACE VEOS CoSim server. "127.0.0.1" if not specified.
     */
    const char* remoteIpAddress;

    /**
     * \brief The name of the dSPACE VEOS CoSim server.
     *        Either the serverName or the remotePort must be specified.
     */
    const char* serverName;

    /**
     * \brief The name of the dSPACE VEOS CoSim Client.
     */
    const char* clientName;

    /**
     * \brief The TCP port of the dSPACE VEOS CoSim server.
     *        Either the serverName or the remotePort must be specified.
     */
    uint16_t remotePort;

    /**
     * \brief The port of the dSPACE VEOS CoSim client.
     *        This value should only be changed if the dSPACE VEOS CoSim communication should be tunneled.
     */
    uint16_t localPort;
} DsVeosCoSim_ConnectConfig;

/**
 * \brief Set the log callback.
 * \param logCallback   The log callback to which all the log messages will be sent.
 */
DSVEOSCOSIM_DECL void DsVeosCoSim_SetLogCallback(DsVeosCoSim_LogCallback logCallback);

/**
 * \brief Creates a handle.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Handle DsVeosCoSim_Create(void);

/**
 * \brief Destroys the given handle.
 * \param handle    The handle to destroy.
 */
DSVEOSCOSIM_DECL void DsVeosCoSim_Destroy(DsVeosCoSim_Handle handle);

/**
 * \brief Connects the given handle to a dSPACE VEOS CoSim server.
 * \param handle        The handle.
 * \param connectConfig The data used for connecting.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_Connect(DsVeosCoSim_Handle handle,
                                                        DsVeosCoSim_ConnectConfig connectConfig);

/**
 * \brief Disconnects the given handle from a dSPACE VEOS CoSim server.
 * \param handle    The handle.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_Disconnect(DsVeosCoSim_Handle handle);

/**
 * \brief Gets a value indicating whether the given handle is connected to a dSPACE VEOS CoSim server.
 * \param handle            The handle.
 * \param connectionState   The connection state as an out value.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetConnectionState(DsVeosCoSim_Handle handle,
                                                                   DsVeosCoSim_ConnectionState* connectionState);

/**
 * \brief Runs a callback based co-simulation for the given handle.
 *        This function will only return if DsVeosCoSim_Disconnect is called in one of the callbacks
 *        or the dSPACE VEOS CoSim server is unloaded.
 * \param handle    The handle.
 * \param callbacks The callbacks.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_RunCallbackBasedCoSimulation(DsVeosCoSim_Handle handle,
                                                                             DsVeosCoSim_Callbacks callbacks);

/**
 * \brief Starts a polling based co-simulation for the given handle.
 * \param handle    The handle.
 * \param callbacks The callbacks.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_StartPollingBasedCoSimulation(DsVeosCoSim_Handle handle,
                                                                              DsVeosCoSim_Callbacks callbacks);

/**
 * \brief Polls a command for the co-simulation for the given handle.
 * \param handle            The handle.
 * \param simulationTime    The simulation time as an out value.
 * \param command           The command as an out value.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_PollCommand(DsVeosCoSim_Handle handle,
                                                            DsVeosCoSim_SimulationTime* simulationTime,
                                                            DsVeosCoSim_Command* command);

/**
 * \brief Finishes the last polled command.
 * \param handle    The handle.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_FinishCommand(DsVeosCoSim_Handle handle);

/**
 * \brief Sets the next simulation time for the co-simulation running for the given handle.
 * \param handle            The handle.
 * \param simulationTime    The next simulation time.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_SetNextSimulationTime(DsVeosCoSim_Handle handle,
                                                                      DsVeosCoSim_SimulationTime simulationTime);

/**
 * \brief Gets the step size of the dSPACE VEOS CoSim server.
 * \param handle                The handle.
 * \param stepSize              The step size.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetStepSize(DsVeosCoSim_Handle handle,
                                                            DsVeosCoSim_SimulationTime* stepSize);

/**
 * \brief Gets all available incoming signals.
 * \param handle                The handle.
 * \param incomingSignalsCount  The incoming signals count.
 * \param incomingSignals       The incoming signals.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetIncomingSignals(DsVeosCoSim_Handle handle,
                                                                   uint32_t* incomingSignalsCount,
                                                                   const DsVeosCoSim_IoSignal** incomingSignals);

/**
 * \brief Reads a value from the incoming signal of the dSPACE VEOS CoSim server identified by the given handle.
 * \param handle            The handle.
 * \param incomingSignalId  The incoming signal id.
 * \param length            The read length.
 * \param value             The read value.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_ReadIncomingSignal(DsVeosCoSim_Handle handle,
                                                                   DsVeosCoSim_IoSignalId incomingSignalId,
                                                                   uint32_t* length,
                                                                   void* value);

/**
 * \brief Gets all available outgoing signals.
 * \param handle                The handle.
 * \param outgoingSignalsCount  The outgoing signals count.
 * \param outgoingSignals       The outgoing signals.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetOutgoingSignals(DsVeosCoSim_Handle handle,
                                                                   uint32_t* outgoingSignalsCount,
                                                                   const DsVeosCoSim_IoSignal** outgoingSignals);

/**
 * \brief Writes the given value to the outgoing signal of the dSPACE VEOS CoSim server identified by the given handle.
 * \param handle            The handle.
 * \param outgoingSignalId  The IO signal id.
 * \param length            The length of the value to write.
 * \param value             The value to write.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_WriteOutgoingSignal(DsVeosCoSim_Handle handle,
                                                                    DsVeosCoSim_IoSignalId outgoingSignalId,
                                                                    uint32_t length,
                                                                    const void* value);

/**
 * \brief Gets all available CAN controllers.
 * \param handle                The handle.
 * \param canControllersCount   The CAN controllers count.
 * \param canControllers        The CAN controllers.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetCanControllers(DsVeosCoSim_Handle handle,
                                                                  uint32_t* canControllersCount,
                                                                  const DsVeosCoSim_CanController** canControllers);

/**
 * \brief Receives a CAN message from the dSPACE VEOS CoSim server identified by the given handle.
 * \param handle    The handle.
 * \param message   The received CAN message.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_ReceiveCanMessage(DsVeosCoSim_Handle handle,
                                                                  DsVeosCoSim_CanMessage* message);

/**
 * \brief Transmits the given message to the dSPACE VEOS CoSim server identified by the given handle.
 * \param handle    The handle.
 * \param message   The message to transmit.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_TransmitCanMessage(DsVeosCoSim_Handle handle,
                                                                   const DsVeosCoSim_CanMessage* message);

/**
 * \brief Gets all available ethernet controllers.
 * \param handle                The handle.
 * \param ethControllersCount   The ethernet controllers count.
 * \param ethControllers        The ethernet controllers.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetEthControllers(DsVeosCoSim_Handle handle,
                                                                  uint32_t* ethControllersCount,
                                                                  const DsVeosCoSim_EthController** ethControllers);

/**
 * \brief Receives an ethernet message from the dSPACE VEOS CoSim server identified by the given handle.
 * \param handle    The handle.
 * \param message   The received ethernet message.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_ReceiveEthMessage(DsVeosCoSim_Handle handle,
                                                                  DsVeosCoSim_EthMessage* message);

/**
 * \brief Transmits the given ethernet message to the dSPACE VEOS CoSim server identified by the given handle.
 * \param handle    The handle.
 * \param message   The message to transmit.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_TransmitEthMessage(DsVeosCoSim_Handle handle,
                                                                   const DsVeosCoSim_EthMessage* message);

/**
 * \brief Gets all available LIN controllers.
 * \param handle                The handle.
 * \param linControllersCount   The LIN controllers count.
 * \param linControllers        The LIN controllers.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetLinControllers(DsVeosCoSim_Handle handle,
                                                                  uint32_t* linControllersCount,
                                                                  const DsVeosCoSim_LinController** linControllers);

/**
 * \brief Receives a LIN message from the dSPACE VEOS CoSim server identified by the given handle.
 * \param handle    The handle.
 * \param message   The received LIN message.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_ReceiveLinMessage(DsVeosCoSim_Handle handle,
                                                                  DsVeosCoSim_LinMessage* message);

/**
 * \brief Transmits the given LIN message to the dSPACE VEOS CoSim server identified by the given handle.
 * \param handle    The handle.
 * \param message   The message to transmit.
 */
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_TransmitLinMessage(DsVeosCoSim_Handle handle,
                                                                   const DsVeosCoSim_LinMessage* message);

DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_StartSimulation(DsVeosCoSim_Handle handle);
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_StopSimulation(DsVeosCoSim_Handle handle);
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_PauseSimulation(DsVeosCoSim_Handle handle);
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_ContinueSimulation(DsVeosCoSim_Handle handle);
DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_TerminateSimulation(DsVeosCoSim_Handle handle,
                                                                    DsVeosCoSim_TerminateReason terminateReason);

DSVEOSCOSIM_DECL DsVeosCoSim_Result DsVeosCoSim_GetCurrentSimulationTime(DsVeosCoSim_Handle handle,
                                                                         DsVeosCoSim_SimulationTime* simulationTime);

DSVEOSCOSIM_DECL const char* DsVeosCoSim_ResultToString(DsVeosCoSim_Result result);
DSVEOSCOSIM_DECL const char* DsVeosCoSim_CommandToString(DsVeosCoSim_Command command);
DSVEOSCOSIM_DECL const char* DsVeosCoSim_SeverityToString(DsVeosCoSim_Severity severity);
DSVEOSCOSIM_DECL const char* DsVeosCoSim_TerminateReasonToString(DsVeosCoSim_TerminateReason terminateReason);
DSVEOSCOSIM_DECL const char* DsVeosCoSim_ConnectionStateToString(DsVeosCoSim_ConnectionState connectionState);
DSVEOSCOSIM_DECL const char* DsVeosCoSim_DataTypeToString(DsVeosCoSim_DataType dataType);
DSVEOSCOSIM_DECL const char* DsVeosCoSim_SizeKindToString(DsVeosCoSim_SizeKind sizeKind);
DSVEOSCOSIM_DECL const char* DsVeosCoSim_LinControllerTypeToString(DsVeosCoSim_LinControllerType linControllerType);
DSVEOSCOSIM_DECL size_t DsVeosCoSim_GetDataTypeSize(DsVeosCoSim_DataType dataType);

#ifdef __cplusplus
extern DSVEOSCOSIM_API std::string DsVeosCoSim_SimulationTimeToString(DsVeosCoSim_SimulationTime simulationTime);
extern DSVEOSCOSIM_API std::string DsVeosCoSim_IoSignalToString(const DsVeosCoSim_IoSignal& ioSignal);
extern DSVEOSCOSIM_API std::string DsVeosCoSim_CanControllerToString(const DsVeosCoSim_CanController& controller);
extern DSVEOSCOSIM_API std::string DsVeosCoSim_EthControllerToString(const DsVeosCoSim_EthController& controller);
extern DSVEOSCOSIM_API std::string DsVeosCoSim_LinControllerToString(const DsVeosCoSim_LinController& controller);
extern DSVEOSCOSIM_API std::string DsVeosCoSim_ValueToString(DsVeosCoSim_DataType dataType,
                                                             uint32_t length,
                                                             const void* value);
extern DSVEOSCOSIM_API std::string DsVeosCoSim_DataToString(const uint8_t* data, size_t dataLength, char separator = 0);
extern DSVEOSCOSIM_API std::string DsVeosCoSim_IoDataToString(const DsVeosCoSim_IoSignal& ioSignal,
                                                              uint32_t length,
                                                              const void* value);
extern DSVEOSCOSIM_API std::string DsVeosCoSim_CanMessageToString(const DsVeosCoSim_CanMessage& message);
extern DSVEOSCOSIM_API std::string DsVeosCoSim_EthMessageToString(const DsVeosCoSim_EthMessage& message);
extern DSVEOSCOSIM_API std::string DsVeosCoSim_LinMessageToString(const DsVeosCoSim_LinMessage& message);
extern DSVEOSCOSIM_API std::string DsVeosCoSim_CanMessageFlagsToString(DsVeosCoSim_CanMessageFlags flags);
extern DSVEOSCOSIM_API std::string DsVeosCoSim_EthMessageFlagsToString(DsVeosCoSim_EthMessageFlags flags);
extern DSVEOSCOSIM_API std::string DsVeosCoSim_LinMessageFlagsToString(DsVeosCoSim_LinMessageFlags flags);

#endif

// NOLINTEND
