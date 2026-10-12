#ifndef _INTEGER_RAY_FRAMEWORK_ANIMATION_FIELD_ANIMATOR_h
#define _INTEGER_RAY_FRAMEWORK_ANIMATION_FIELD_ANIMATOR_h

#include "AnimatorCore.h"

namespace IntegerRay
{
	namespace Animation
	{
		template<typename field_t>
		class FieldAnimator : public AnimatorCore<field_t>
		{
		private:
			field_t* Field = nullptr;

		protected:
			FieldAnimator()
				: AnimatorCore<field_t>()
			{}

			void OnValueChanged(const uint32_t, const field_t value, const scalar_t) override
			{
				if (Field != nullptr)
				{
					*Field = value;
				}
			}

		public:
			FieldAnimator(field_t& field)
				: AnimatorCore<field_t>()
				, Field(&field)
			{}

			FieldAnimator(const FieldAnimator&) = delete;
			FieldAnimator& operator=(const FieldAnimator&) = delete;
			FieldAnimator(FieldAnimator&&) = delete;
			FieldAnimator& operator=(FieldAnimator&&) = delete;

			void SetField(field_t& field)
			{
				Field = &field;
			}

			void AnimateTo(const uint32_t duration, const field_t endValue)
			{
				this->SetDuration(duration);
				this->Start(endValue, *Field);
			}

			void AnimateFromTo(const uint32_t duration, const field_t startValue, const field_t endValue)
			{
				this->SetDuration(duration);
				*Field = startValue;
				this->Start(endValue, startValue);
			}

		};

		template<typename field_t>
		class TemplateAnimatedField : public FieldAnimator<field_t>
		{
		public:
			field_t Value{};

		public:
			TemplateAnimatedField()
			{
				this->SetField(Value);
			}
			~TemplateAnimatedField() = default;

			void SetValue(const field_t value)
			{
				Value = value;
			}

			field_t GetValue() const
			{
				return Value;
			}
		};
	}
}

#endif
