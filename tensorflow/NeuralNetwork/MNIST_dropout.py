import tensorflow as tf
from tensorflow.examples.tutorials.mnist import input_data
mnist = input_data.read_data_sets("MNIST_data/", one_hot= True)

nb_classes = 10 # from 0 to 9

X = tf.placeholder(tf.float32, [None, 28*28]) # 28*28 pixels
Y = tf.placeholder(tf.float32, [None, nb_classes])
keep_prob = tf.placeholder(tf.float32)
#layer1
W1 = tf.get_variable("W1",shape= [28*28, 512])
b1 = tf.Variable(tf.random_normal([512]))
L1 = tf.nn.relu(tf.matmul(X, W1) + b1)
L1 = tf.nn.dropout(L1, keep_prob= keep_prob)
#layer2
W2 = tf.get_variable("W2",shape= [512, 512])
b2 = tf.Variable(tf.random_normal([512]))
L2 = tf.nn.relu(tf.matmul(L1, W2) + b2)
L2 = tf.nn.dropout(L2, keep_prob= keep_prob)
#layer3
W3 = tf.get_variable("W3",shape= [512, 512])
b3 = tf.Variable(tf.random_normal([512]))
L3 = tf.nn.relu(tf.matmul(L2, W3) + b3)
L1 = tf.nn.dropout(L3, keep_prob= keep_prob)
#layer4
W4 = tf.get_variable("W4",shape= [512, 512])
b4 = tf.Variable(tf.random_normal([512]))
L4 = tf.nn.relu(tf.matmul(L3, W4) + b4)
L4 = tf.nn.dropout(L4, keep_prob= keep_prob)
#layer5
W5 = tf.get_variable("W5", shape= [512, 10])
b5 = tf.Variable(tf.random_normal([10]))
hypothesis = tf.matmul(L4, W5) + b5

cost = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(logits= hypothesis, labels= Y))
train = tf.train.AdamOptimizer(learning_rate= 0.001).minimize(cost)

is_correct = tf.equal(tf.arg_max(hypothesis, 1), tf.arg_max(Y, 1))
accuracy = tf.reduce_mean(tf.cast(is_correct, tf.float32))

training_epochs = 15
batch_size = 100

with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    for epoch in range(training_epochs):
        total_batch = int(mnist.train.num_examples / batch_size)
        avg_cost = 0
        for iter in range(total_batch):
            batch_xs, batch_ys = mnist.train.next_batch(batch_size)
            c, _ = sess.run([cost, train], feed_dict= {X: batch_xs, Y: batch_ys, keep_prob: 0.7})
            avg_cost += c / total_batch
        print('Epoch:', '%04d' % (epoch + 1), 'cost = ', '{:.9f}'.format(avg_cost))
    print('Accuracy = ', accuracy.eval(session= sess, feed_dict={X: mnist.test.images, Y: mnist.test.labels, keep_prob: 1.0}))
