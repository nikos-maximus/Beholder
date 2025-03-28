#ifndef BH_PIPELINE_HPP
#define BH_PIPELINE_HPP

////////////////////////////////////////////////////////////////////////////////
struct bhMaterial
{
public:
protected:
private:
};

class bhPipeline
{
public:
  //enum class Type
  //{
  //  PIPELINE_WORLD,

  //  NUM_PIPELINE_TYPES
  //};

protected:
private:
};

////////////////////////////////////////////////////////////////////////////////
struct bhWorldMaterial : public bhMaterial
{
public:
protected:
private:
};

class bhWorldPipeline : public bhPipeline
{
public:
  virtual ~bhWorldPipeline() = default;

protected:
private:
};

#endif //BH_PIPELINE_HPP
