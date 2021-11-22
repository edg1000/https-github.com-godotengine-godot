using Godot;
using NUnit.Framework;

namespace GodotSharp.UnitTest
{
    public class Vector3iTests
    {
        [SetUp]
        public void Setup()
        {
        }

        [Test]
        public void Vector3i_constructor()
        {
            var vector3i = new Vector3i(1, 2, 3);
            Assert.AreEqual(1.1, vector3i.x);
            Assert.AreEqual(2.2, vector3i.y);
            Assert.AreEqual(3.3, vector3i.z);
        }

        [Test]
        public void Vector3i_constructor_from_other()
        {
            var vector3i = new Vector3i(1, 2, 3);
            var vector3iCopy = new Vector3i(vector3i);
            Assert.AreNotSame(vector3i, vector3iCopy);
            Assert.AreEqual(vector3i.x, vector3iCopy.x);
            Assert.AreEqual(vector3i.y, vector3iCopy.y);
            Assert.AreEqual(vector3i.z, vector3iCopy.z);
        }

        [Test]
        public void Vector3i_constructor_from_Vector3()
        {
            var vector3i = new Vector3i(new Vector3(1.1f, 2.6f, 3.9f));
            Assert.AreEqual(1, vector3i.x);
            Assert.AreEqual(3, vector3i.y);
            Assert.AreEqual(4, vector3i.z);
        }

        [Test]
        public void Vector3i_xyz()
        {
            var vector3i = new Vector3i(1, 2, 3);
            var vector3iCopy = vector3i.xyz;
            Assert.AreNotSame(vector3i, vector3iCopy);
            Assert.AreEqual(vector3i.x, vector3iCopy.x);
            Assert.AreEqual(vector3i.y, vector3iCopy.y);
            Assert.AreEqual(vector3i.z, vector3iCopy.z);
        }

        [Test]
        public void Vector3i_xzy()
        {
            var vector3i = new Vector3i(1, 2, 3);
            var vector3iCopy = vector3i.xzy;
            Assert.AreNotSame(vector3i, vector3iCopy);
            Assert.AreEqual(vector3i.x, vector3iCopy.x);
            Assert.AreEqual(vector3i.z, vector3iCopy.y);
            Assert.AreEqual(vector3i.y, vector3iCopy.z);
        }

        [Test]
        public void Vector3i_yxz()
        {
            var vector3i = new Vector3i(1, 2, 3);
            var vector3iCopy = vector3i.yxz;
            Assert.AreNotSame(vector3i, vector3iCopy);
            Assert.AreEqual(vector3i.y, vector3iCopy.x);
            Assert.AreEqual(vector3i.x, vector3iCopy.y);
            Assert.AreEqual(vector3i.z, vector3iCopy.z);
        }

        [Test]
        public void Vector3i_yzx()
        {
            var vector3i = new Vector3i(1, 2, 3);
            var vector3iCopy = vector3i.yzx;
            Assert.AreNotSame(vector3i, vector3iCopy);
            Assert.AreEqual(vector3i.y, vector3iCopy.x);
            Assert.AreEqual(vector3i.z, vector3iCopy.y);
            Assert.AreEqual(vector3i.x, vector3iCopy.z);
        }

        [Test]
        public void Vector3i_zxy()
        {
            var vector3i = new Vector3i(1, 2, 3);
            var vector3iCopy = vector3i.zxy;
            Assert.AreNotSame(vector3i, vector3iCopy);
            Assert.AreEqual(vector3i.z, vector3iCopy.x);
            Assert.AreEqual(vector3i.x, vector3iCopy.y);
            Assert.AreEqual(vector3i.y, vector3iCopy.z);
        }

        [Test]
        public void Vector3i_zyx()
        {
            var vector3i = new Vector3i(1, 2, 3);
            var vector3iCopy = vector3i.zyx;
            Assert.AreNotSame(vector3i, vector3iCopy);
            Assert.AreEqual(vector3i.z, vector3iCopy.x);
            Assert.AreEqual(vector3i.y, vector3iCopy.y);
            Assert.AreEqual(vector3i.x, vector3iCopy.z);
        }

        [Test]
        public void Vector3i_xy()
        {
            var vector3i = new Vector3i(1, 2, 3);
            var vector3iCopy = vector3i.xy;
            Assert.AreNotSame(vector3i, vector3iCopy);
            Assert.AreEqual(vector3i.x, vector3iCopy.x);
            Assert.AreEqual(vector3i.y, vector3iCopy.y);
        }

        [Test]
        public void Vector3i_xz()
        {
            var vector3i = new Vector3i(1, 2, 3);
            var vector3iCopy = vector3i.xz;
            Assert.AreNotSame(vector3i, vector3iCopy);
            Assert.AreEqual(vector3i.x, vector3iCopy.x);
            Assert.AreEqual(vector3i.z, vector3iCopy.y);
        }

        [Test]
        public void Vector3i_yx()
        {
            var vector3i = new Vector3i(1, 2, 3);
            var vector3iCopy = vector3i.yx;
            Assert.AreNotSame(vector3i, vector3iCopy);
            Assert.AreEqual(vector3i.y, vector3iCopy.x);
            Assert.AreEqual(vector3i.x, vector3iCopy.y);
        }

        [Test]
        public void Vector3i_yz()
        {
            var vector3i = new Vector3i(1, 2, 3);
            var vector3iCopy = vector3i.yz;
            Assert.AreNotSame(vector3i, vector3iCopy);
            Assert.AreEqual(vector3i.y, vector3iCopy.x);
            Assert.AreEqual(vector3i.z, vector3iCopy.y);
        }

        [Test]
        public void Vector3i_zx()
        {
            var vector3i = new Vector3i(1, 2, 3);
            var vector3iCopy = vector3i.zx;
            Assert.AreNotSame(vector3i, vector3iCopy);
            Assert.AreEqual(vector3i.z, vector3iCopy.x);
            Assert.AreEqual(vector3i.x, vector3iCopy.y);
        }

        [Test]
        public void Vector3i_zy()
        {
            var vector3i = new Vector3i(1, 2, 3);
            var vector3iCopy = vector3i.zy;
            Assert.AreNotSame(vector3i, vector3iCopy);
            Assert.AreEqual(vector3i.z, vector3iCopy.x);
            Assert.AreEqual(vector3i.y, vector3iCopy.y);
        }
    }
}
