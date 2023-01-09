#include <chrono>
namespace legit
{
  class GpuProfiler
  {
  public:
    GpuProfiler(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, uint32_t maxTimestampsCount) :
      logicalDevice(logicalDevice),
      timestampQuery(physicalDevice, logicalDevice, maxTimestampsCount)
    {
      frameIndex = 0;
    }
    size_t StartTask(std::string taskName, uint32_t taskColor, vk::PipelineStageFlagBits pipelineStageFlags)
    {
      timestampQuery.AddTimestamp(frameCommandBuffer, profilerTasks.size(), pipelineStageFlags);

      legit::ProfilerTask task;
      task.color = taskColor;
      task.name = taskName;
      task.startTime = -1.0;
      task.endTime = -1.0;
      size_t taskId = profilerTasks.size();
      profilerTasks.push_back(task);


      return taskId;
    }
    void EndTask(size_t taskId)
    {
      assert(profilerTasks.size() == taskId + 1 && profilerTasks.back().endTime < 0.0);
    }

    size_t StartFrame(vk::CommandBuffer commandBuffer)
    {
      this->frameCommandBuffer = commandBuffer;
      profilerTasks.clear();
      timestampQuery.ResetQueryPool(frameCommandBuffer);
      return frameIndex;
    }
    void EndFrame(size_t frameId)
    {
      timestampQuery.AddTimestamp(frameCommandBuffer, profilerTasks.size(), vk::PipelineStageFlagBits::eBottomOfPipe);

      assert(frameId == frameIndex);
      frameIndex++;
    }
    const std::vector<ProfilerTask> &GetProfilerTasks()
    {
      return profilerTasks;
    }
  private:

    struct TaskHandleInfo
    {
      TaskHandleInfo(GpuProfiler *_profiler, size_t _taskId)
      {
        this->profiler = _profiler;
        this->taskId = _taskId;
      }
      void Reset()
      {
        profiler->EndTask(taskId);
      }
      GpuProfiler *profiler;
      size_t taskId;
    };
    struct FrameHandleInfo
    {
      FrameHandleInfo(GpuProfiler *_profiler, size_t _frameId)
      {
        this->profiler = _profiler;
        this->frameId = _frameId;
      }
      void Reset()
      {
        profiler->EndFrame(frameId);
      }
      GpuProfiler *profiler;
      size_t frameId;
    };
  public:
    using ScopedTask = UniqueHandle<TaskHandleInfo, GpuProfiler>;
    ScopedTask StartScopedTask(std::string taskName, uint32_t taskColor, vk::PipelineStageFlagBits pipelineStageFlags)
    {
      return ScopedTask(TaskHandleInfo(this, StartTask(taskName, taskColor, pipelineStageFlags)), true);
    }
    using ScopedFrame = UniqueHandle<FrameHandleInfo, GpuProfiler>;
    ScopedFrame StartScopedFrame(vk::CommandBuffer commandBuffer)
    {
      return ScopedFrame(FrameHandleInfo(this, StartFrame(commandBuffer)), true);
    }

    const std::vector<legit::ProfilerTask> &GetProfilerData()
    {
      return profilerTasks;
    }
    void GatherTimestamps()
    {
      if (profilerTasks.size() > 0)
      {
        legit::TimestampQuery::QueryResult res = timestampQuery.QueryResults(logicalDevice);
        assert(res.size == this->profilerTasks.size() + 1); //1 is because of end-of-frame timestamp

        for (size_t taskIndex = 0; taskIndex < profilerTasks.size(); taskIndex++)
        {
          auto &task = profilerTasks[taskIndex];
          task.startTime = res.data[taskIndex].time;
          task.endTime = res.data[taskIndex + 1].time;
        }
      }
    }
  private:
    vk::Device logicalDevice;
    TimestampQuery timestampQuery;
    size_t frameIndex;
    std::vector<legit::ProfilerTask> profilerTasks;
    vk::CommandBuffer frameCommandBuffer;
    friend struct UniqueHandle<TaskHandleInfo, GpuProfiler>;
  };
}